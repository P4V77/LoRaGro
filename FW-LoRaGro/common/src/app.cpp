#include "app.hpp"

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

/* ---- Device tree bindings ---- */

static const struct device *const envi_dev =
    DEVICE_DT_GET(DT_ALIAS(envirmoental_sensor));

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
      lora_transceiver_(lora_dev),
      auth_(dev_cfg_),
      tx_codec_(cfg_),
      rx_handler_(cfg_),
      pwr_mgr_(sample_mgr_, loragro::SensorID::BATTERY_VOLTAGE, dev_cfg_),
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

    LOG_INF("Device ID: %d",
            loragro::extract_node(dev_cfg_.combined_id));

    LOG_INF("Assigned Gateway ID: %d",
            loragro::extract_gateway(dev_cfg_.combined_id));

    register_sensors();

    return 0;
}

/* ========================================================= */
/* ================= Register Sensors ====================== */
/* ========================================================= */

int loragro::App::register_sensors()
{
    LOG_INF("Registering sensors");

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
    regulator_.powerOn();

    sample_mgr_.init_all();
    lora_transceiver_.init(dev_cfg_);

    sample_mgr_.sample_all();
    auto batch = sample_mgr_.get_batch();

    tx_codec_.begin(batch);

    uint8_t frame[255]{};
    size_t max_payload = lora_transceiver_.get_max_payload();
    size_t usable_payload = max_payload - FrameLayout::AUTH_SIZE;

    while (tx_codec_.has_frame_to_send())
    {
        int len = tx_codec_.build_frame(frame, usable_payload);
        if (len <= 0)
            break;

        if (auth_.sign_frame(frame, usable_payload, max_payload) < 0)
            LOG_ERR("Frame Auth Failed");

        uint8_t frame_nmbr = tx_codec_.get_frame_number(frame, len);

        int ret = lora_transceiver_.send_confirmed(frame, len);
        if (ret < 0)
            LOG_ERR("Frame %d failed: %d", frame_nmbr, ret);
        else
            LOG_INF("Frame %d ACKed", frame_nmbr);
    }

    while (true)
    {
        int received = lora_transceiver_.receive(frame, max_payload);
        if (received <= 0)
            break;

        const DecodeResult result =
            rx_handler_.decode(frame, received);

        const int len =
            tx_codec_.build_frame(frame, usable_payload, result);

        if (len <= 0)
            break;

        lora_transceiver_.send_response(frame, len);
    }

    regulator_.powerOff();

    cfg_.save();
    pwr_mgr_.handle_sleep();
}
