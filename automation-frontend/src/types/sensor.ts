export interface LocationData {
  location: string
  displayName: string
  temperature?: {
    current: number
    lastUpdated: string
    status: 'online' | 'offline' | 'error'
  }
  humidity?: {
    current: number
    lastUpdated: string
    status: 'online' | 'offline' | 'error'
  }
}

export interface ChartDataPoint {
  x: string
  y: number
}

export interface SensorAlert {
  type: 'temperature' | 'humidity' | 'offline'
  location: string
  message: string
  severity: 'low' | 'medium' | 'high'
}

export type TemperatureRange = 'cold' | 'cool' | 'comfortable' | 'warm' | 'hot'

export interface TemperatureRangeConfig {
  range: TemperatureRange
  min: number
  max: number
  color: string
  bgColor: string
  label: string
}
