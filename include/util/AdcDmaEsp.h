/**
 * @file AdcDmaEsp.h
 * @author josuemorais
 * @version 1.0
 * @date 2023-08-30
 *
 * @brief Driver para leitura de ADC via I2S com DMA no ESP32
 *
 * Esta biblioteca permite a leitura de dados analógicos usando o periférico I2S
 * com DMA para aquisição de alta performance. Utiliza o ADC1 e fornece os dados
 * através de um sistema de callback.
 */

#ifndef I2SADC_H
#define I2SADC_H

#include <Arduino.h>
#include <driver/i2s.h>
#include <esp_adc_cal.h>
/**
 * @brief Tipo de função de callback para processamento de dados
 *
 * @param data Ponteiro para o array de dados ADC (12 bits)
 * @param count Número de amostras no array
 */
typedef void (*ADCCallback)(const uint16_t *data, size_t count);

/**
 * @brief Classe principal para gerenciamento do ADC via I2S
 *
 * Exemplo de uso:
 * @code
 * I2SADC adc;
 *
 * void setup() {
 *   adc.begin();
 *   adc.setCallback(meuCallback);
 * }
 *
 * void loop() {
 *   adc.update();
 * }
 * @endcode
 */
class AdcDmaEsp
{
public:
    /**
     * @brief Constrói um novo objeto AdcDmaEsp
     *
     * @param channel Canal ADC1 a ser utilizado (padrão: ADC1_CHANNEL_0)
     * @param sample_rate Taxa de amostragem em Hz (padrão: 44100)
     * @param dma_buffers Número de buffers DMA (padrão: 4)
     * @param buffer_len Tamanho de cada buffer DMA (padrão: 64)
     */
    AdcDmaEsp(adc1_channel_t channel, uint32_t sample_rate, int dma_buffers, int buffer_len); // Tipo alterado

    /**
     * @brief Inicializa o hardware e aloca recursos
     *
     * @return true se inicialização bem sucedida
     * @return false em caso de falha (alocação de memória, etc)
     */
    bool begin();

    /**
     * @brief Define a função de callback para processamento de dados
     *
     * @param callback Função a ser chamada quando novos dados estiverem disponíveis
     */
    void setCallback(ADCCallback callback);

    /**
     * @brief Atualiza o estado do driver e chama o callback quando necessário
     *
     * Deve ser chamado regularmente no loop principal
     */
    void update();

    /**
     * @brief Para a aquisição e libera recursos
     */
    void stop();

    ~AdcDmaEsp();

private:
    adc1_channel_t _channel;
    uint32_t _sample_rate;
    int _dma_buffers; // Tipo compatível com i2s_config_t
    int _buffer_len;  // Tipo compatível com i2s_config_t
    ADCCallback _callback;
    uint16_t *_dma_buffer;
    bool _initialized;

    void configureI2S();
    void configureADC();
};

#endif

/**
 * @brief Constrói um novo objeto I2SADC
 *
 * @param channel Canal ADC1 (0-7)
 * @param sample_rate Taxa de amostragem (Hz)
 * @param dma_buffers Número de buffers DMA
 * @param buffer_len Tamanho de cada buffer
 */
AdcDmaEsp::AdcDmaEsp(adc1_channel_t channel = ADC1_CHANNEL_3,
                     uint32_t sample_rate = 1000,
                     int dma_buffers = 4,
                     int buffer_len = 64) : _channel(channel),
                                            _sample_rate(sample_rate),
                                            _dma_buffers(dma_buffers),
                                            _buffer_len(buffer_len),
                                            _callback(nullptr),
                                            _dma_buffer(nullptr),
                                            _initialized(false) {}

AdcDmaEsp::~AdcDmaEsp()
{
    stop();
}

/**
 * @brief Inicializa o hardware e configura os periféricos
 *
 * @return bool true se bem sucedido
 */
bool AdcDmaEsp::begin()
{
    if (_initialized)
        return true;

    // Alinha memória para requisitos DMA
    _dma_buffer = (uint16_t *)aligned_alloc(16, _dma_buffers * _buffer_len * sizeof(uint16_t));
    if (!_dma_buffer)
        return false;

    configureADC();
    configureI2S();

    _initialized = true;
    return true;
}

/**
 * @brief Configura o ADC1 com os parâmetros especificados
 */
void AdcDmaEsp::configureADC()
{
    adc_power_acquire();
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(_channel, ADC_ATTEN_DB_12);
}

/**
 * @brief Configura o periférico I2S para modo ADC
 */
void AdcDmaEsp::configureI2S()
{
    i2s_config_t i2s_config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
        .sample_rate = static_cast<uint32_t>(_sample_rate),
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = static_cast<int>(_dma_buffers), // Conversão explícita
        .dma_buf_len = static_cast<int>(_buffer_len),    // Conversão explícita
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0,
        .mclk_multiple = I2S_MCLK_MULTIPLE_256};

    if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK)
    {
        Serial.println("Failed to install I2S driver");
        return;
    }

    i2s_set_adc_mode(ADC_UNIT_1, _channel);
    i2s_adc_enable(I2S_NUM_0);
}

/**
 * @brief Define a função de callback para processamento de dados
 *
 * @param callback Função a ser chamada com novos dados
 */
void AdcDmaEsp::setCallback(ADCCallback callback)
{
    _callback = callback;
}

/**
 * @brief Atualiza o driver e processa dados disponíveis
 */
void AdcDmaEsp::update()
{
    if (!_initialized || !_callback)
        return;

    size_t bytes_read;
    esp_err_t err = i2s_read(I2S_NUM_0, _dma_buffer,
                             _dma_buffers * _buffer_len * sizeof(int16_t),
                             &bytes_read, 0);

    if (err == ESP_OK && bytes_read > 0)
    {
        size_t samples = bytes_read / sizeof(int16_t);
        _callback(_dma_buffer, samples);
    }
}

/**
 * @brief Para a aquisição e libera recursos
 */
void AdcDmaEsp::stop()
{
    if (_initialized)
    {
        i2s_adc_disable(I2S_NUM_0);
        i2s_driver_uninstall(I2S_NUM_0);
        free(_dma_buffer);
        _initialized = false;
    }
}