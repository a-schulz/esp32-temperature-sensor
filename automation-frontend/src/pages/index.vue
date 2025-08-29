<template>
  <div class="dashboard">
    <!-- Loading State -->
    <v-row v-if="loading" justify="center" class="my-12">
      <v-col cols="12" class="text-center">
        <v-progress-circular
          indeterminate
          size="64"
          width="6"
          color="primary"
        />
        <div class="text-h5 mt-4">Sensordaten werden geladen...</div>
      </v-col>
    </v-row>

    <!-- Error State -->
    <v-alert
      v-if="error && !loading"
      type="error"
      prominent
      border="start"
      class="mb-6"
    >
      <template #prepend>
        <v-icon size="32">mdi-alert-circle</v-icon>
      </template>
      <div class="text-h6">Fehler beim Laden der Daten</div>
      <div class="text-body-1 mt-2">{{ error }}</div>
      <template #append>
        <v-btn
          color="error"
          variant="outlined"
          @click="fetchSensorData"
          class="ml-4"
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
          :type="alert.severity === 'high' ? 'error' : 'warning'"
          prominent
          border="start"
          class="mb-3"
        >
          <template #prepend>
            <v-icon size="24">
              {{ alert.type === 'offline' ? 'mdi-wifi-off' : 'mdi-thermometer-alert' }}
            </v-icon>
          </template>
          <div class="text-body-1 font-weight-medium">{{ alert.message }}</div>
        </v-alert>
      </v-col>
    </v-row>

    <!-- Current Values Section -->
    <v-row v-if="!loading && processedData.length > 0">
      <v-col cols="12" class="mb-4">
        <div class="text-h4 mb-6">
          <v-icon class="mr-3" color="primary">mdi-home-thermometer</v-icon>
          Aktuelle Werte
        </div>
      </v-col>

      <v-col
        v-for="location in processedData"
        :key="location.location"
        cols="12"
        md="6"
        lg="4"
      >
        <v-card
          class="sensor-card"
          :class="{ 'elevation-8': true }"
          height="280"
        >
          <v-card-title class="text-h5 pb-2">
            <v-icon class="mr-2" color="primary">mdi-map-marker</v-icon>
            {{ location.displayName }}
          </v-card-title>

          <v-card-text>
            <!-- Temperature Display -->
            <div v-if="location.temperature" class="mb-4">
              <div class="d-flex align-center justify-space-between mb-2">
                <div class="text-h6">
                  <v-icon class="mr-1">mdi-thermometer</v-icon>
                  Temperatur
                </div>
                <v-chip
                  :color="location.temperature.status === 'online' ? 'success' : 'error'"
                  size="small"
                  variant="flat"
                >
                  {{ location.temperature.status === 'online' ? 'Online' : 'Offline' }}
                </v-chip>
              </div>
              
              <div class="d-flex align-center">
                <div 
                  class="temperature-value mr-3"
                  :style="{ color: getTemperatureRange(location.temperature.current).color }"
                >
                  {{ location.temperature.current.toFixed(1) }}°C
                </div>
                <v-chip
                  :color="getTemperatureRange(location.temperature.current).color"
                  :style="{ backgroundColor: getTemperatureRange(location.temperature.current).bgColor }"
                  size="large"
                  variant="flat"
                >
                  {{ getTemperatureRange(location.temperature.current).label }}
                </v-chip>
              </div>
              
              <div class="text-caption mt-1 text-medium-emphasis">
                {{ formatRelativeTime(location.temperature.lastUpdated) }}
              </div>
            </div>

            <!-- Humidity Display -->
            <div v-if="location.humidity">
              <div class="d-flex align-center justify-space-between mb-2">
                <div class="text-h6">
                  <v-icon class="mr-1">mdi-water-percent</v-icon>
                  Luftfeuchtigkeit
                </div>
                <v-chip
                  :color="location.humidity.status === 'online' ? 'success' : 'error'"
                  size="small"
                  variant="flat"
                >
                  {{ location.humidity.status === 'online' ? 'Online' : 'Offline' }}
                </v-chip>
              </div>
              
              <div class="humidity-value" style="color: #388E3C;">
                {{ location.humidity.current.toFixed(0) }}%
              </div>
              
              <div class="text-caption mt-1 text-medium-emphasis">
                {{ formatRelativeTime(location.humidity.lastUpdated) }}
              </div>
            </div>

            <!-- No Data State -->
            <div v-if="!location.temperature && !location.humidity" class="text-center py-4">
              <v-icon size="48" color="grey">mdi-database-off</v-icon>
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
      <v-col cols="12" class="mb-4">
        <div class="text-h4 mb-6">
          <v-icon class="mr-3" color="primary">mdi-chart-line</v-icon>
          Verlauf der letzten 24 Stunden
        </div>
        
        <!-- Time Range Selection -->
        <v-btn-toggle
          v-model="selectedTimeRange"
          mandatory
          class="mb-6"
          variant="outlined"
          divided
        >
          <v-btn value="24" class="text-body-1" size="large">
            24 Stunden
          </v-btn>
          <v-btn value="168" class="text-body-1" size="large">
            7 Tage
          </v-btn>
          <v-btn value="720" class="text-body-1" size="large">
            30 Tage
          </v-btn>
        </v-btn-toggle>
      </v-col>

      <!-- Temperature Charts -->
      <v-col
        v-for="location in processedData.filter(l => l.temperature)"
        :key="`temp-${location.location}`"
        cols="12"
        lg="6"
      >
        <v-card class="sensor-card">
          <v-card-title class="text-h6">
            <v-icon class="mr-2" color="primary">mdi-thermometer</v-icon>
            {{ location.displayName }}
          </v-card-title>
          
          <v-card-text>
            <div v-if="chartLoading" class="text-center py-8">
              <v-progress-circular indeterminate color="primary" />
              <div class="text-body-1 mt-2">Daten werden geladen...</div>
            </div>
            
            <SensorChart
              v-else-if="chartData[location.location]?.temperature && chartData[location.location].temperature!.length > 0"
              :data="chartData[location.location].temperature!"
              type="temperature"
              :height="300"
            />
            
            <div v-else class="text-center py-8">
              <v-icon size="48" color="grey">mdi-chart-line-variant</v-icon>
              <div class="text-body-1 mt-2 text-medium-emphasis">
                Keine Diagrammdaten verfügbar
              </div>
            </div>
          </v-card-text>
        </v-card>
      </v-col>

      <!-- Humidity Charts -->
      <v-col
        v-for="location in processedData.filter(l => l.humidity)"
        :key="`humidity-${location.location}`"
        cols="12"
        lg="6"
      >
        <v-card class="sensor-card">
          <v-card-title class="text-h6">
            <v-icon class="mr-2" color="success">mdi-water-percent</v-icon>
            {{ location.displayName }}
          </v-card-title>
          
          <v-card-text>
            <div v-if="chartLoading" class="text-center py-8">
              <v-progress-circular indeterminate color="success" />
              <div class="text-body-1 mt-2">Daten werden geladen...</div>
            </div>
            
            <SensorChart
              v-else-if="chartData[location.location]?.humidity && chartData[location.location].humidity!.length > 0"
              :data="chartData[location.location].humidity!"
              type="humidity"
              :height="300"
            />
            
            <div v-else class="text-center py-8">
              <v-icon size="48" color="grey">mdi-chart-line-variant</v-icon>
              <div class="text-body-1 mt-2 text-medium-emphasis">
                Keine Diagrammdaten verfügbar
              </div>
            </div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <!-- No Data State -->
    <v-row v-if="!loading && processedData.length === 0" justify="center" class="my-12">
      <v-col cols="12" md="8" lg="6" class="text-center">
        <v-icon size="96" color="grey">mdi-database-off-outline</v-icon>
        <div class="text-h4 mt-4 mb-2">Keine Sensordaten gefunden</div>
        <div class="text-body-1 text-medium-emphasis mb-4">
          Es sind derzeit keine Temperatur- oder Feuchtigkeitsdaten verfügbar.
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

    <!-- Refresh Button (Always Visible) -->
    <v-fab
      location="bottom end"
      size="large"
      color="primary"
      icon="mdi-refresh"
      @click="handleRefresh"
      :loading="loading"
      class="ma-4"
    />
  </div>
</template>

<script lang="ts" setup>
import { ref, computed, watch, onMounted } from 'vue'
import { useSensorData } from '@/composables/useSensorData'
import SensorChart from '@/components/SensorChart.vue'
import type { EnvironmentMeasurement } from '@/types/sensor'

// Use the sensor data composable
const {
  processedData,
  alerts,
  loading,
  error,
  fetchSensorData,
  getHistoricalData,
  getTemperatureRange,
  formatRelativeTime
} = useSensorData()

// Chart data state
const chartData = ref<Record<string, { temperature?: EnvironmentMeasurement[], humidity?: EnvironmentMeasurement[] }>>({})
const chartLoading = ref(false)
const selectedTimeRange = ref('24') // 24 hours default

// Load chart data for all locations
const loadChartData = async () => {
  if (processedData.value.length === 0) return
  
  chartLoading.value = true
  const newChartData: typeof chartData.value = {}
  
  try {
    const hours = parseInt(selectedTimeRange.value)
    
    // Load data for each location
    for (const location of processedData.value) {
      newChartData[location.location] = {}
      
      // Load temperature data if available
      if (location.temperature) {
        const tempData = await getHistoricalData(location.location, 'temperature', hours)
        newChartData[location.location].temperature = tempData
      }
      
      // Load humidity data if available
      if (location.humidity) {
        const humidityData = await getHistoricalData(location.location, 'humidity', hours)
        newChartData[location.location].humidity = humidityData
      }
    }
    
    chartData.value = newChartData
  } catch (err) {
    console.error('Error loading chart data:', err)
  } finally {
    chartLoading.value = false
  }
}

// Handle refresh with user feedback
const handleRefresh = async () => {
  await fetchSensorData()
  await loadChartData()
}

// Watch for changes in processed data and time range
watch([processedData, selectedTimeRange], () => {
  loadChartData()
}, { deep: true })

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
  transition: transform 0.2s ease-in-out, box-shadow 0.2s ease-in-out;
  
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
