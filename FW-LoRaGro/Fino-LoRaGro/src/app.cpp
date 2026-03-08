#include "app.hpp"

LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

/* ---- Device tree bindings ---- */

static const struct device *const envi_dev =
    DEVICE_DT_GET(DT_ALIAS(environmental_sensor));

static const struct device *const light_dev =
    DEVICE_DT_GET(DT_ALIAS(light_sensor));

static const struct device *const co2_dev =
    DEVICE_DT_GET(DT_ALIAS(co2_sensor));

static const struct device *const soil_modbus_dev =
    DEVICE_DT_GET(DT_ALIAS(soil_sensor));

static const struct device *const adc_dev =
    DEVICE_DT_GET(DT_ALIAS(adc0));

static const struct device *const lora_dev =
    DEVICE_DT_GET(DT_ALIAS(lora0));

/* ========================================================= */
/* ===================== Constructor ======================= */
/* ========================================================= */

loragro::App::App()
    : cfg_(ConfigManager::instance()),
      auth_(cfg_.get()),
      lora_transceiver_(lora_dev, cfg_.get(), auth_),
      tx_codec_(cfg_),
      rx_handler_(cfg_),
      pwr_mgr_(sample_mgr_, loragro::SensorID::BATTERY_VOLTAGE, cfg_.get()),
      env_sensor_(envi_dev,
                  SensorID::ENV_TEMP,
                  SensorID::ENV_RH,
                  SensorID::ENV_PRESS),
      light_sensor_(light_dev, SensorID::AMB_LIGHT),
      co2_sensor_(co2_dev,
                  SensorID::CO2_CONC,
                  SensorID::CO2_TEMP,
                  SensorID::CO2_RH),
      soil_3in1_sensor_(soil_modbus_dev,
                        SensorID::SOIL_MOISTURE,
                        SensorID::SOIL_TEMP,
                        SensorID::SOIL_EC),
      soil_analog_sensor_(adc_dev,
                          SensorID::SOIL_ANALOG_MOISTURE),
      battery_sense_(adc_dev,
                     SensorID::BATTERY_VOLTAGE)
{
}

/* ========================================================= */
/* ======================== Init =========================== */
/* ========================================================= */

int loragro::App::init()
{
    cfg_.load();
    dev_cfg_ = cfg_.get();

    LOG_DBG("Device ID: %d",
            loragro::extract_node(dev_cfg_.combined_id));

    LOG_DBG("Assigned Gateway ID: %d",
            loragro::extract_gateway(dev_cfg_.combined_id));

    register_sensors();

    /* Check lora_dev if ready */
    if (!device_is_ready(lora_dev))
        return -ENODEV;

    return 0;
}

/* ========================================================= */
/* ================= Register Sensors ====================== */
/* ========================================================= */

int loragro::App::register_sensors()
{
    LOG_DBG("Registering sensors");

    if (device_is_ready(envi_dev))
        sample_mgr_.add_sensor(&env_sensor_);

    if (device_is_ready(light_dev))
        sample_mgr_.add_sensor(&light_sensor_);

    if (device_is_ready(co2_dev))
        sample_mgr_.add_sensor(&co2_sensor_);

    if (device_is_ready(soil_modbus_dev))
        sample_mgr_.add_sensor(&soil_3in1_sensor_);

    if (device_is_ready(adc_dev) && soil_analog_sensor_.is_connected())
        sample_mgr_.add_sensor(&soil_analog_sensor_);

    if (device_is_ready(adc_dev) && battery_sense_.is_connected())
        sample_mgr_.add_sensor(&battery_sense_);

    return 0;
}

/* ========================================================= */
/* ========================= Run =========================== */
/* ========================================================= */

void loragro::App::run()
{
    /* Sleeping for calculated time offset
     * Every node has its own time slot based on id
     * slot calculated on mcu start with 50% margin */
    while (true)
    {
        run_cycle();
    }
}

/* ========================================================= */
/* ====================== Run Cycle ======================== */
/* ========================================================= */

void loragro::App::run_cycle()
{
    cfg_.load();
    dev_cfg_ = cfg_.get();
    regulator_.powerOn();

    sample_mgr_.init_all();
    lora_transceiver_.init(dev_cfg_);
    auth_.init_key();

    sample_mgr_.sample_all();
    auto batch = sample_mgr_.get_batch();

    tx_codec_.begin(batch);

    std::array<uint8_t, 255> au8Frame;
    size_t max_payload = lora_transceiver_.get_max_payload();
    size_t usable_payload = max_payload - FrameLayout::AUTH_SIZE;

    uint8_t sent_frame_count = 0;
    while (tx_codec_.has_frame_to_send())
    {
        if (sent_frame_count > dev_cfg_.max_tx_frames_per_cycle)
        {
            LOG_ERR("More than 3 frames (21 measurements) can't be sent, TDMA window cant allow");
            LOG_ERR("Scraping last frame(s)");
            break;
        }

        au8Frame.fill(0);

        int len = tx_codec_.build_frame(au8Frame.begin(), usable_payload);
        if (len <= 0)
            break;

        if (auth_.sign_frame(au8Frame.begin(), len, max_payload) < 0)
            LOG_ERR("Frame Auth Failed");

        uint8_t frame_nmbr = tx_codec_.get_frame_number(au8Frame.begin(), len);
        len += FrameLayout::AUTH_SIZE;
        int ret = lora_transceiver_.send_confirmed(au8Frame.begin(), len);
        if (ret < 0)
            LOG_ERR("Frame %d failed: %d", frame_nmbr, ret);
        else
            LOG_DBG("Frame %d ACKed", frame_nmbr);

        sent_frame_count++;
    }

    /* Only 1 RX for each sleep cycle */
    au8Frame.fill(0);

    int received = lora_transceiver_.receive(au8Frame.begin(), max_payload);
    if (received > 0)
    {
        const uint8_t *tag = au8Frame.begin() + (received - FrameLayout::AUTH_SIZE);
        const uint8_t data_len = (received - FrameLayout::AUTH_SIZE);
        const uint8_t frame_ctr = au8Frame[FrameLayout::FRAME_CTR];

        LOG_DBG("RX received=%d, frame_ctr=%u, frame_type=0x%02x",
                received, frame_ctr, au8Frame[FrameLayout::FRAME_TYPE]);

        int verify_ret = auth_.verify_frame(au8Frame.begin(),
                                            data_len,
                                            frame_ctr,
                                            tag);

        if (verify_ret != 0)
            LOG_ERR("RX Frame Verify Auth Failed");

        // LOG_DBG("verify_frame ret=%d", verify_ret);
        const DecodeResult result = rx_handler_.decode(au8Frame.begin(), received);
        // LOG_DBG("decode result cmd=%d", static_cast<int>(result));

        const int len = tx_codec_.build_frame(au8Frame.begin(), usable_payload, result);
        if (len > 0)
        {
            if (auth_.sign_frame(au8Frame.begin(), len, max_payload) < 0)
                LOG_ERR("Response Frame Auth Failed");
            else
            {
                lora_transceiver_.send_response(au8Frame.begin(), len);
                LOG_HEXDUMP_INF(au8Frame.begin(), len + 4, "Respond:");
            }
        }
    }

    regulator_.powerOff();

    cfg_.save();
    LOG_DBG("\n\n");
    pwr_mgr_.handle_sleep();
}
