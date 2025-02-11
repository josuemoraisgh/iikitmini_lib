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

#ifndef ADCDMAESP_H
#define ADCDMAESP_H

#include <driver/i2s.h>

// Número de buffers DMA (padrão: 4)
#ifndef DMA_BUFFERS
#define DMA_BUFFERS 4
#endif

// Tamanho de cada buffer (em amostras; padrão: 64)
#ifndef BUFFER_LEN
#define BUFFER_LEN 64
#endif

/**
 * @brief Tipo de callback para tratar os dados ADC.
 *
 * O callback deve receber um ponteiro para os dados de 16 bits lidos e o número de amostras.
 */
typedef void (*ADCCallback)(const int16_t *data, size_t count);

/** Callback que será invocado quando os dados forem adquiridos */
ADCCallback _adcDmaEspcallback;

/**
 * @brief Buffer DMA para armazenamento dos dados lidos.
 *
 * Alinhado a 16 bytes para compatibilidade com o DMA.
 */
__attribute__((aligned(16))) int16_t dma_buffer[DMA_BUFFERS * BUFFER_LEN];

/** Intervalo de plotagem em milissegundos */
uint32_t _plotInterval = 0;
/** Última vez que os dados foram plotados (millis) */
uint32_t _last_plot = 0;

/**
 * @brief Configura o ADC via DMA utilizando o I2S built-in do ESP32.
 *
 * Esta função configura o ADC (ADC1) para utilizar um canal específico, define a largura dos bits,
 * a atenuação, e instala o driver I2S no modo ADC built-in. Após a instalação, o ADC é habilitado.
 *
 * @param channel Canal ADC (do tipo adc1_channel_t) a ser utilizado (ex: ADC1_CHANNEL_0, ADC1_CHANNEL_3, etc.).
 * @param callback Função de callback que será invocada com os dados adquiridos.
 * @param width_bit Largura dos bits para conversão ADC (padrão ADC_WIDTH_BIT_12).
 * @param sample_rate Taxa de amostragem em Hz (padrão: 1000 Hz).
 * @param plotInterval Intervalo em milissegundos para invocar o callback (padrão: 100 ms).
 */
void adcDmaSetup(adc1_channel_t channel, ADCCallback callback, adc_bits_width_t width_bit = ADC_WIDTH_BIT_12, uint32_t sample_rate = 1000, uint32_t plotInterval = 100)
{
  _adcDmaEspcallback = callback;
  _plotInterval = plotInterval;

  // Adquire energia para o ADC
  adc_power_acquire();

  // Configura a largura do ADC
  adc1_config_width(width_bit);
  // Configura a atenuação para o canal
  adc1_config_channel_atten(channel, ADC_ATTEN_DB_12);

  // Configuração do I2S para operação com ADC via DMA
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
      .sample_rate = sample_rate,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = DMA_BUFFERS,
      .dma_buf_len = BUFFER_LEN,
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0,
      .mclk_multiple = I2S_MCLK_MULTIPLE_256};

  // Instala o driver I2S e verifica o resultado
  if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK)
  {
    Serial.println("ERR:I2S_INIT");
    while (1)
      ;
  }

  // Configura o ADC para usar o canal especificado (ADC_UNIT_1 e o canal informado)
  i2s_set_adc_mode(ADC_UNIT_1, channel);
  // Habilita o ADC via I2S
  i2s_adc_enable(I2S_NUM_0);
}

/**
 * @brief Loop de aquisição e callback.
 *
 * Esta função deve ser chamada periodicamente no loop principal. Ela lê os dados do ADC
 * via I2S utilizando DMA e, se o intervalo de plotagem tiver decorrido, invoca o callback com os dados.
 */
void adcDmaLoop()
{
  // Se o intervalo de plotagem foi atingido, chama o callback com os dados lidos
  if (millis() - _last_plot >= _plotInterval)
  {
    size_t bytes_read;
    esp_err_t err = i2s_read(I2S_NUM_0, dma_buffer, sizeof(dma_buffer), &bytes_read, 0);
    if (err == ESP_OK)
      _adcDmaEspcallback(dma_buffer, (uint16_t)(bytes_read / sizeof(int16_t)));
    _last_plot = millis();
  }
}

#endif // ADCDMAESP_H
