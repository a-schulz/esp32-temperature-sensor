<template>
  <div class="dashboard">
    <!-- Loading State -->
    <v-row v-if="loading" class="my-12" justify="center">
      <v-col class="text-center" cols="12">
        <v-progress-circular
          color="primary"
          indeterminate
          size="64"
          width="6"
        />
        <div class="text-h5 mt-4">Sensordaten werden geladen...</div>
      </v-col>
    </v-row>

    <!-- Error State -->
    <v-alert
      v-if="error && !loading"
      border="start"
      class="mb-6"
      prominent
      type="error"
    >
      <template #prepend>
        <v-icon size="32">mdi-alert-circle</v-icon>
      </template>
      <div class="text-h6">Fehler beim Laden der Daten</div>
      <div class="text-body-1 mt-2">{{ error }}</div>
      <template #append>
        <v-btn
          class="ml-4"
          color="error"
          variant="outlined"
          @click="fetchSensorData"
        >
          Erneut versuchen
        </v-btn>
      </template>
    </v-alert>

    <!-- Alerts Section -->
    <v-row v-if="alerts.length > 0 && !loading">
      <v-col cols="12">
        <div class="text-h5 mb-4">
          <v-icon class="mr-2" color="warning">mdi-alert</v-icon>
          Warnungen
        </div>
        <v-alert
          v-for="alert in alerts"
          :key="`${alert.location}-${alert.type}`"
          border="start"
          class="mb-3"
          prominent
          :type="alert.severity === 'high' ? 'error' : 'warning'"
        >
          <template #prepend>
            <v-icon size="24">
              {{
                alert.type === 'offline'
                  ? 'mdi-wifi-off'
                  : 'mdi-thermometer-alert'
              }}
            </v-icon>
          </template>
          <div class="text-body-1 font-weight-medium">
            {{ alert.message }}
          </div>
        </v-alert>
      </v-col>
    </v-row>

    <!-- Current Values Section -->
    <v-row v-if="!loading && processedData.length > 0">
      <v-col cols="12">
        <v-row>
          <v-col cols="10">
            <div class="text-h4">
              <v-icon class="mr-3" color="primary">
                mdi-home-thermometer
              </v-icon>
              Aktuelle Werte
            </div>
          </v-col>
          <v-col cols="2">
            <v-btn
              icon="mdi-refresh"
              :loading="loading"
              location="bottom end"
              variant="tonal"
              @click="fetchSensorData"
            />
          </v-col>
        </v-row>
      </v-col>
      <v-col
        v-for="location in processedData"
        :key="location.location"
        cols="12"
        lg="4"
        md="6"
      >
        <v-card class="sensor-card">
          <v-card-title class="text-h5 pb-2">
            <!-- was: <v-icon class="mr-2" color="primary">mdi-map-marker</v-icon> -->
            <v-icon class="mr-2" color="primary">
              {{ getLocationIcon(location.displayName) }}
            </v-icon>
            {{ location.displayName }}
          </v-card-title>

          <v-card-text>
            <!-- Temperature Display -->
            <div v-if="location.temperature" class="mb-4">
              <div
                class="d-flex align-center justify-space-between mb-2"
              >
                <div class="text-h6">
                  <v-icon class="mr-1">
                    mdi-thermometer
                  </v-icon>
                  Temperatur
                </div>
                <v-chip
                  :color="
                    location.temperature.status === 'online'
                      ? 'success'
                      : 'error'
                  "
                  size="small"
                  variant="flat"
                >
                  {{
                    location.temperature.status === 'online'
                      ? 'Online'
                      : 'Offline'
                  }}
                </v-chip>
              </div>

              <div class="d-flex align-center">
                <div
                  class="temperature-value mr-3"
                  :style="{
                    color: getTemperatureRange(
                      location.temperature.current
                    ).color,
                  }"
                >
                  {{
                    location.temperature.current.toFixed(1)
                  }}°C
                </div>
                <v-chip
                  :color="
                    getTemperatureRange(
                      location.temperature.current
                    ).color
                  "
                  size="large"
                  :style="{
                    backgroundColor: getTemperatureRange(
                      location.temperature.current
                    ).bgColor,
                  }"
                >
                  {{
                    getTemperatureRange(
                      location.temperature.current
                    ).label
                  }}
                </v-chip>
              </div>

              <div class="text-caption mt-1 text-medium-emphasis">
                {{
                  formatRelativeTime(
                    location.temperature.lastUpdated
                  )
                }}
              </div>
            </div>

            <!-- Humidity Display -->
            <div v-if="location.humidity">
              <div
                class="d-flex align-center justify-space-between mb-2"
              >
                <div class="text-h6">
                  <v-icon class="mr-1">
                    mdi-water-percent
                  </v-icon>
                  Luftfeuchtigkeit
                </div>
                <v-chip
                  :color="
                    location.humidity.status === 'online'
                      ? 'success'
                      : 'error'
                  "
                  size="small"
                  variant="flat"
                >
                  {{
                    location.humidity.status === 'online'
                      ? 'Online'
                      : 'Offline'
                  }}
                </v-chip>
              </div>

              <div class="humidity-value" style="color: #388e3c">
                {{ location.humidity.current.toFixed(0) }}%
              </div>

              <div class="text-caption mt-1 text-medium-emphasis">
                {{
                  formatRelativeTime(
                    location.humidity.lastUpdated
                  )
                }}
              </div>
            </div>

            <!-- No Data State -->
            <div
              v-if="!location.temperature && !location.humidity"
              class="text-center py-4"
            >
              <v-icon color="grey" size="48">
                mdi-database-off
              </v-icon>
              <div class="text-body-1 mt-2 text-medium-emphasis">
                Keine Daten verfügbar
              </div>
            </div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <!-- Charts Section -->
    <v-row v-if="!loading && processedData.length > 0" class="mt-8">
      <v-col class="mb-4" cols="12">
        <div class="text-h4 mb-6">
          <v-icon class="mr-3" color="primary">mdi-chart-line</v-icon>
          Verlauf der letzten
          <span v-if="selectedTimeRange === '24'">24 Stunden</span>
          <span v-else-if="selectedTimeRange === '168'">7 Tage</span>
          <span v-else-if="selectedTimeRange === '720'">30 Tage</span>
        </div>

        <!-- Time Range Selection -->
        <v-btn-toggle
          v-model="selectedTimeRange"
          class="mb-6"
          divided
          mandatory
          variant="outlined"
        >
          <v-btn class="text-body-1" size="large" value="24">
            24 Stunden
          </v-btn>
          <v-btn class="text-body-1" size="large" value="168">
            7 Tage
          </v-btn>
          <v-btn class="text-body-1" size="large" value="720">
            30 Tage
          </v-btn>
        </v-btn-toggle>
      </v-col>

      <!-- Temperature Charts -->
      <v-col
        v-for="location in processedData.filter((l) => l.temperature)"
        :key="`temp-${location.location}`"
        cols="12"
        lg="6"
      >
        <v-card class="sensor-card">
          <v-card-title class="text-h6">
            <v-icon class="mr-2" color="primary">
              mdi-thermometer
            </v-icon>
            {{ location.displayName }}
          </v-card-title>

          <v-card-text>
            <div v-if="chartLoading" class="text-center py-8">
              <v-progress-circular
                color="primary"
                indeterminate
              />
              <div class="text-body-1 mt-2">
                Daten werden geladen...
              </div>
            </div>

            <SensorChart
              v-else-if="
                chartData[location.location]?.temperature &&
                  chartData[location.location].temperature!
                    .length > 0
              "
              :data="chartData[location.location].temperature!"
              :height="300"
              type="temperature"
            />

            <div v-else class="text-center py-8">
              <v-icon color="grey" size="48">
                mdi-chart-line-variant
              </v-icon>
              <div class="text-body-1 mt-2 text-medium-emphasis">
                Keine Diagrammdaten verfügbar
              </div>
            </div>
          </v-card-text>
        </v-card>
      </v-col>

      <!-- Humidity Charts -->
      <v-col
        v-for="location in processedData.filter((l) => l.humidity)"
        :key="`humidity-${location.location}`"
        cols="12"
        lg="6"
      >
        <v-card class="sensor-card">
          <v-card-title class="text-h6">
            <v-icon class="mr-2" color="success">
              mdi-water-percent
            </v-icon>
            {{ location.displayName }}
          </v-card-title>

          <v-card-text>
            <div v-if="chartLoading" class="text-center py-8">
              <v-progress-circular
                color="success"
                indeterminate
              />
              <div class="text-body-1 mt-2">
                Daten werden geladen...
              </div>
            </div>

            <SensorChart
              v-else-if="
                chartData[location.location]?.humidity &&
                  chartData[location.location].humidity!.length >
                  0
              "
              :data="chartData[location.location].humidity!"
              :height="300"
              type="humidity"
            />

            <div v-else class="text-center py-8">
              <v-icon color="grey" size="48">
                mdi-chart-line-variant
              </v-icon>
              <div class="text-body-1 mt-2 text-medium-emphasis">
                Keine Diagrammdaten verfügbar
              </div>
            </div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <!-- No Data State -->
    <v-row
      v-if="!loading && processedData.length === 0"
      class="my-12"
      justify="center"
    >
      <v-col class="text-center" cols="12" lg="6" md="8">
        <v-icon color="grey" size="96">mdi-database-off-outline</v-icon>
        <div class="text-h4 mt-4 mb-2">Keine Sensordaten gefunden</div>
        <div class="text-body-1 text-medium-emphasis mb-4">
          Es sind derzeit keine Temperatur- oder Feuchtigkeitsdaten
          verfügbar.
        </div>
        <v-btn
          color="primary"
          size="large"
          variant="outlined"
          @click="fetchSensorData"
        >
          <v-icon class="mr-2">mdi-refresh</v-icon>
          Aktualisieren
        </v-btn>
      </v-col>
    </v-row>
  </div>
</template>

<script lang="ts" setup>
  import type { EnvironmentMeasurement } from '@/types/sensor'
  import { computed, onMounted, ref, watch } from 'vue'
  import SensorChart from '@/components/SensorChart.vue'
  import { useSensorData } from '@/composables/useSensorData'

  // Use the sensor data composable
  const {
    processedData,
    alerts,
    loading,
    error,
    fetchSensorData,
    getHistoricalData,
    getTemperatureRange,
    formatRelativeTime,
  } = useSensorData()

  // Pick an icon by searching for relevant keywords in the location name.
  // Avoids a static map and tries to find the best match.
  function getLocationIcon (name: string): string {
    const n = (name || '').toLowerCase().trim()

    if (/(living|wohn|lounge|stube)/.test(n)) return 'mdi-sofa'
    if (/(sleep|bed|schlaf)/.test(n)) return 'mdi-bed-queen'
    if (/(kitchen|küche|kueche|cook)/.test(n))
      return 'mdi-silverware-fork-knife'
    if (/(office|büro|buero|study|arbeits)/.test(n)) return 'mdi-desk'
    if (/(basement|keller|cellar)/.test(n)) return 'mdi-stairs-down'
    if (/(attic|dachboden)/.test(n)) return 'mdi-home-roof'
    if (/garage/.test(n)) return 'mdi-garage'
    if (/\b(bath|bad|bathroom|wc|toilet)\b/.test(n)) return 'mdi-shower'
    if (/(garden|garten|yard|outdoor|terrasse|terrace|patio)/.test(n))
      return 'mdi-tree'
    if (/(balcony|balkon)/.test(n)) return 'mdi-flower'
    if (/(server|rack)/.test(n)) return 'mdi-server'
    if (/(nursery|kinder|kids|child)/.test(n))
      return 'mdi-baby-face-outline'
    if (/(hall|flur|corridor|gang)/.test(n)) return 'mdi-door-open'
    if (/(dining|essen|speise)/.test(n)) return 'mdi-silverware'
    if (/(storage|abstell|speicher)/.test(n)) return 'mdi-archive-outline'
    if (/(heating|heizung|boiler)/.test(n)) return 'mdi-heating-coil'

    // As a generic indoor space, prefer a home icon; otherwise fall back to marker
    if (/(room|zimmer|home|haus)/.test(n)) return 'mdi-home-outline'
    return 'mdi-map-marker'
  }

  // Chart data state
  const chartData = ref<
    Record<
      string,
      {
        temperature?: EnvironmentMeasurement[]
        humidity?: EnvironmentMeasurement[]
      }
    >
  >({})
  const chartLoading = ref(false)
  const selectedTimeRange = ref('24') // 24 hours default

  // Load chart data for all locations
  async function loadChartData () {
    if (processedData.value.length === 0) return

    chartLoading.value = true
    const newChartData: typeof chartData.value = {}

    try {
      const hours = Number.parseInt(selectedTimeRange.value)

      // Load data for each location
      for (const location of processedData.value) {
        newChartData[location.location] = {}

        // Load temperature data if available
        if (location.temperature) {
          const tempData = await getHistoricalData(
            location.location,
            'temperature',
            hours,
          )
          newChartData[location.location].temperature = tempData
        }

        // Load humidity data if available
        if (location.humidity) {
          const humidityData = await getHistoricalData(
            location.location,
            'humidity',
            hours,
          )
          newChartData[location.location].humidity = humidityData
        }
      }

      chartData.value = newChartData
    } catch (error_) {
      console.error('Error loading chart data:', error_)
    } finally {
      chartLoading.value = false
    }
  }

  // Handle refresh with user feedback
  async function handleRefresh () {
    await fetchSensorData()
    await loadChartData()
  }

  // Watch for changes in processed data and time range
  watch(
    [processedData, selectedTimeRange],
    () => {
      loadChartData()
    },
    { deep: true },
  )

  // Watch for time range changes
  watch(selectedTimeRange, () => {
    loadChartData()
  })

  // Load chart data on mount
  onMounted(() => {
    // Small delay to ensure processed data is ready
    setTimeout(() => {
      loadChartData()
    }, 1000)
  })
</script>

<style lang="scss" scoped>
    .dashboard {
        max-width: 1400px;
        margin: 0 auto;
    }

    .sensor-card {
        transition:
            transform 0.2s ease-in-out,
            box-shadow 0.2s ease-in-out;

        &:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 16px rgba(0, 0, 0, 0.15) !important;
        }
    }

    .v-btn-toggle {
        .v-btn {
            min-width: 120px;
            font-size: 1rem;
        }
    }

    // Enhanced focus styles for accessibility
    .v-btn:focus-visible {
        outline: 3px solid rgba(25, 118, 210, 0.5);
        outline-offset: 2px;
    }

    .v-card:focus-within {
        outline: 2px solid rgba(25, 118, 210, 0.3);
        outline-offset: 2px;
    }

    // Mobile responsiveness
    @media (max-width: 960px) {
        .temperature-value {
            font-size: 2.5rem !important;
        }

        .humidity-value {
            font-size: 2rem !important;
        }

        .sensor-card {
            margin: 8px 0;
        }
    }

    @media (max-width: 600px) {
        .v-btn-toggle {
            width: 100%;

            .v-btn {
                flex: 1;
                min-width: auto;
            }
        }
    }
</style>
