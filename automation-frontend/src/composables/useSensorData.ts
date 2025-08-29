import { ref, computed, onMounted, onUnmounted, readonly } from 'vue'
import { supabase } from '@/utils/supabase'
import type { 
  EnvironmentMeasurement, 
  LocationData, 
  SensorAlert, 
  TemperatureRange, 
  TemperatureRangeConfig 
} from '@/types/sensor'

export function useSensorData() {
  // Reactive state
  const measurements = ref<EnvironmentMeasurement[]>([])
  const loading = ref(true)
  const error = ref<string | null>(null)
  const lastUpdate = ref<Date | null>(null)
  
  // Auto-refresh interval
  let refreshInterval: NodeJS.Timeout | null = null
  
  // Temperature ranges configuration (German labels)
  const temperatureRanges: TemperatureRangeConfig[] = [
    { range: 'cold', min: -50, max: 16, color: '#1976D2', bgColor: '#E3F2FD', label: 'Kalt' },
    { range: 'cool', min: 16, max: 19, color: '#0288D1', bgColor: '#E1F5FE', label: 'Kühl' },
    { range: 'comfortable', min: 19, max: 24, color: '#388E3C', bgColor: '#E8F5E8', label: 'Angenehm' },
    { range: 'warm', min: 24, max: 27, color: '#F57C00', bgColor: '#FFF3E0', label: 'Warm' },
    { range: 'hot', min: 27, max: 50, color: '#D32F2F', bgColor: '#FFEBEE', label: 'Heiß' }
  ]
  
  // Location display names (German)
  const locationDisplayNames: Record<string, string> = {
    garage: 'Garage',
    heating: 'Heizung'
  }
  
  /**
   * Get temperature range for a given temperature value
   */
  const getTemperatureRange = (temperature: number): TemperatureRangeConfig => {
    return temperatureRanges.find(range => 
      temperature >= range.min && temperature < range.max
    ) || temperatureRanges[2] // Default to comfortable
  }
  
  /**
   * Format relative time in German
   */
  const formatRelativeTime = (dateString: string): string => {
    const now = new Date()
    const then = new Date(dateString)
    const diffMs = now.getTime() - then.getTime()
    const diffMinutes = Math.floor(diffMs / (1000 * 60))
    const diffHours = Math.floor(diffMs / (1000 * 60 * 60))
    
    if (diffMinutes < 1) return 'Gerade eben'
    if (diffMinutes < 60) return `vor ${diffMinutes} Minuten`
    if (diffHours < 24) return `vor ${diffHours} Stunden`
    
    const diffDays = Math.floor(diffMs / (1000 * 60 * 60 * 24))
    return `vor ${diffDays} Tagen`
  }
  
  /**
   * Check if sensor data is considered offline (>5 minutes old)
   */
  const isOffline = (dateString: string): boolean => {
    const SENSOR_SUBMIT_INTERVAL = 15
    const OFFLINE_THRESHOLD = SENSOR_SUBMIT_INTERVAL + 1 // minutes
    const now = new Date()
    const then = new Date(dateString)
    const diffMs = now.getTime() - then.getTime()
    return diffMs > OFFLINE_THRESHOLD * 60 * 1000
  }
  
  /**
   * Process raw measurements into location-based data structure
   */
  const processedData = computed<LocationData[]>(() => {
    const locationMap = new Map<string, LocationData>()
    
    // Group measurements by location
    measurements.value.forEach(measurement => {
      if (!locationMap.has(measurement.location)) {
        locationMap.set(measurement.location, {
          location: measurement.location,
          displayName: locationDisplayNames[measurement.location] || measurement.location
        })
      }
      
      const locationData = locationMap.get(measurement.location)!
      const status = isOffline(measurement.created_at) ? 'offline' : 'online'
      
      if (measurement.type === 'temperature') {
        locationData.temperature = {
          current: measurement.value,
          lastUpdated: measurement.created_at,
          status
        }
      } else if (measurement.type === 'humidity') {
        locationData.humidity = {
          current: measurement.value,
          lastUpdated: measurement.created_at,
          status
        }
      }
    })
    
    return Array.from(locationMap.values())
  })
  
  /**
   * Generate alerts based on current sensor data
   */
  const alerts = computed<SensorAlert[]>(() => {
    const alertList: SensorAlert[] = []
    
    processedData.value.forEach(location => {
      // Temperature alerts
      if (location.temperature) {
        if (location.temperature.status === 'offline') {
          alertList.push({
            type: 'offline',
            location: location.displayName,
            message: `Temperatursensor in ${location.displayName} ist offline`,
            severity: 'medium'
          })
        } else {
          // Different temperature thresholds based on location
          const isHeatingLocation = location.location === 'heating'
          const lowThreshold = 10
          const highThreshold = isHeatingLocation ? 80 : 30 // Allow heating to go up to 80°C
          
          if (location.temperature.current < lowThreshold) {
            alertList.push({
              type: 'temperature',
              location: location.displayName,
              message: `Sehr niedrige Temperatur in ${location.displayName}: ${location.temperature.current.toFixed(1)}°C`,
              severity: 'high'
            })
          } else if (location.temperature.current > highThreshold) {
            alertList.push({
              type: 'temperature',
              location: location.displayName,
              message: `Sehr hohe Temperatur in ${location.displayName}: ${location.temperature.current.toFixed(1)}°C`,
              severity: 'high'
            })
          }
        }
      }
      
      // Humidity alerts
      if (location.humidity && location.humidity.status === 'offline') {
        alertList.push({
          type: 'offline',
          location: location.displayName,
          message: `Feuchtigkeitssensor in ${location.displayName} ist offline`,
          severity: 'medium'
        })
      }
    })
    
    return alertList
  })
  
  /**
   * Fetch latest sensor data from Supabase
   */
  const fetchSensorData = async (): Promise<void> => {
    try {
      loading.value = true
      error.value = null
      
      // Get latest measurement for each location/type combination
    const { data: latestData, error: fetchError } = await supabase
      .from('environment_measurements')
      .select('*')
      .in('location', ['garage', 'heating'])
      .order('created_at', { ascending: false })
      
      if (fetchError) throw fetchError
      
      // Keep only the latest measurement for each location/type combination
      const uniqueMeasurements = new Map<string, EnvironmentMeasurement>()
      
      latestData?.forEach(measurement => {
        const key = `${measurement.location}-${measurement.type}`
        if (!uniqueMeasurements.has(key)) {
          uniqueMeasurements.set(key, measurement)
        }
      })
      
      measurements.value = Array.from(uniqueMeasurements.values())
      lastUpdate.value = new Date()
      
    } catch (err) {
      console.error('Error fetching sensor data:', err)
      error.value = 'Fehler beim Laden der Sensordaten'
    } finally {
      loading.value = false
    }
  }
  
  /**
   * Get historical data for charts
   */
  const getHistoricalData = async (location: string, type: 'temperature' | 'humidity', hours: number = 24) => {
    try {
      const startTime = new Date()
      startTime.setHours(startTime.getHours() - hours)
      
      const { data, error: fetchError } = await supabase
        .from('environment_measurements')
        .select('*')
        .eq('location', location)
        .eq('type', type)
        .gte('created_at', startTime.toISOString())
        .order('created_at', { ascending: true })
      
      if (fetchError) throw fetchError
      
      return data || []
      
    } catch (err) {
      console.error('Error fetching historical data:', err)
      return []
    }
  }
  
  /**
   * Start auto-refresh interval
   */
  const startAutoRefresh = (): void => {
    if (refreshInterval) clearInterval(refreshInterval)
    
    refreshInterval = setInterval(() => {
      fetchSensorData()
    }, 30000) // 30 seconds
  }
  
  /**
   * Stop auto-refresh interval
   */
  const stopAutoRefresh = (): void => {
    if (refreshInterval) {
      clearInterval(refreshInterval)
      refreshInterval = null
    }
  }
  
  // Lifecycle hooks
  onMounted(() => {
    fetchSensorData()
    startAutoRefresh()
  })
  
  onUnmounted(() => {
    stopAutoRefresh()
  })
  
  return {
    // State
    measurements: readonly(measurements),
    loading: readonly(loading),
    error: readonly(error),
    lastUpdate: readonly(lastUpdate),
    
    // Computed
    processedData,
    alerts,
    
    // Methods
    fetchSensorData,
    getHistoricalData,
    getTemperatureRange,
    formatRelativeTime,
    isOffline,
    
    // Configuration
    temperatureRanges,
    locationDisplayNames
  }
}