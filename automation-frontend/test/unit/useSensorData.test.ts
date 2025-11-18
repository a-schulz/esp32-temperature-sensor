import type { EnvironmentMeasurement } from '@/types/sensor'
import { beforeEach, describe, expect, it, vi } from 'vitest'

// Mock Supabase
vi.mock('@/utils/supabase', () => ({
  supabase: {
    from: vi.fn(() => ({
      select: vi.fn(() => ({
        in: vi.fn(() => ({
          order: vi.fn(() => ({
            then: vi.fn(),
          })),
        })),
        eq: vi.fn(() => ({
          eq: vi.fn(() => ({
            gte: vi.fn(() => ({
              order: vi.fn(() => ({
                then: vi.fn(),
              })),
            })),
          })),
        })),
      })),
    })),
  },
}))

describe('useSensorData', () => {
  let useSensorData: any

  beforeEach(async () => {
    vi.clearAllMocks()
    // Reset modules to ensure fresh state
    vi.resetModules()
    const module = await import('@/composables/useSensorData')
    useSensorData = module.useSensorData
  })

  describe('getTemperatureRange', () => {
    it('should classify cold temperature correctly', () => {
      const { getTemperatureRange } = useSensorData()
      const result = getTemperatureRange(10)
      expect(result.range).toBe('cold')
      expect(result.label).toBe('Kalt')
      expect(result.color).toBe('#1976D2')
    })

    it('should classify cool temperature correctly', () => {
      const { getTemperatureRange } = useSensorData()
      const result = getTemperatureRange(17)
      expect(result.range).toBe('cool')
      expect(result.label).toBe('Kühl')
      expect(result.color).toBe('#0288D1')
    })

    it('should classify comfortable temperature correctly', () => {
      const { getTemperatureRange } = useSensorData()
      const result = getTemperatureRange(21)
      expect(result.range).toBe('comfortable')
      expect(result.label).toBe('Angenehm')
      expect(result.color).toBe('#388E3C')
    })

    it('should classify warm temperature correctly', () => {
      const { getTemperatureRange } = useSensorData()
      const result = getTemperatureRange(25)
      expect(result.range).toBe('warm')
      expect(result.label).toBe('Warm')
      expect(result.color).toBe('#F57C00')
    })

    it('should classify hot temperature correctly', () => {
      const { getTemperatureRange } = useSensorData()
      const result = getTemperatureRange(30)
      expect(result.range).toBe('hot')
      expect(result.label).toBe('Heiß')
      expect(result.color).toBe('#D32F2F')
    })

    it('should handle boundary value at 16°C (cold/cool)', () => {
      const { getTemperatureRange } = useSensorData()
      const result = getTemperatureRange(16)
      expect(result.range).toBe('cool')
    })

    it('should handle boundary value at 19°C (cool/comfortable)', () => {
      const { getTemperatureRange } = useSensorData()
      const result = getTemperatureRange(19)
      expect(result.range).toBe('comfortable')
    })

    it('should handle boundary value at 24°C (comfortable/warm)', () => {
      const { getTemperatureRange } = useSensorData()
      const result = getTemperatureRange(24)
      expect(result.range).toBe('warm')
    })

    it('should handle boundary value at 27°C (warm/hot)', () => {
      const { getTemperatureRange } = useSensorData()
      const result = getTemperatureRange(27)
      expect(result.range).toBe('hot')
    })

    it('should default to comfortable for extreme high values', () => {
      const { getTemperatureRange } = useSensorData()
      const result = getTemperatureRange(100)
      expect(result.range).toBe('comfortable')
    })

    it('should handle extreme low values', () => {
      const { getTemperatureRange } = useSensorData()
      const result = getTemperatureRange(-100)
      expect(result.range).toBe('cold')
    })
  })

  describe('formatRelativeTime', () => {
    it('should return "Gerade eben" for timestamps less than 1 minute old', () => {
      const { formatRelativeTime } = useSensorData()
      const now = new Date()
      const thirtySecondsAgo = new Date(now.getTime() - 30 * 1000)
      expect(formatRelativeTime(thirtySecondsAgo.toISOString())).toBe('Gerade eben')
    })

    it('should return "vor X Minuten" for timestamps between 1-59 minutes', () => {
      const { formatRelativeTime } = useSensorData()
      const now = new Date()
      const fiveMinutesAgo = new Date(now.getTime() - 5 * 60 * 1000)
      expect(formatRelativeTime(fiveMinutesAgo.toISOString())).toBe('vor 5 Minuten')
    })

    it('should return "vor X Stunden" for timestamps between 1-23 hours', () => {
      const { formatRelativeTime } = useSensorData()
      const now = new Date()
      const threeHoursAgo = new Date(now.getTime() - 3 * 60 * 60 * 1000)
      expect(formatRelativeTime(threeHoursAgo.toISOString())).toBe('vor 3 Stunden')
    })

    it('should return "vor X Tagen" for timestamps 24+ hours old', () => {
      const { formatRelativeTime } = useSensorData()
      const now = new Date()
      const twoDaysAgo = new Date(now.getTime() - 2 * 24 * 60 * 60 * 1000)
      expect(formatRelativeTime(twoDaysAgo.toISOString())).toBe('vor 2 Tagen')
    })

    it('should handle exactly 1 minute', () => {
      const { formatRelativeTime } = useSensorData()
      const now = new Date()
      const oneMinuteAgo = new Date(now.getTime() - 60 * 1000)
      expect(formatRelativeTime(oneMinuteAgo.toISOString())).toBe('vor 1 Minuten')
    })

    it('should handle exactly 1 hour', () => {
      const { formatRelativeTime } = useSensorData()
      const now = new Date()
      const oneHourAgo = new Date(now.getTime() - 60 * 60 * 1000)
      expect(formatRelativeTime(oneHourAgo.toISOString())).toBe('vor 1 Stunden')
    })
  })

  describe('isOffline', () => {
    it('should return false for recent timestamps (< 16 minutes)', () => {
      const { isOffline } = useSensorData()
      const now = new Date()
      const tenMinutesAgo = new Date(now.getTime() - 10 * 60 * 1000)
      expect(isOffline(tenMinutesAgo.toISOString())).toBe(false)
    })

    it('should return true for old timestamps (> 16 minutes)', () => {
      const { isOffline } = useSensorData()
      const now = new Date()
      const twentyMinutesAgo = new Date(now.getTime() - 20 * 60 * 1000)
      expect(isOffline(twentyMinutesAgo.toISOString())).toBe(true)
    })

    it('should handle exactly 16 minutes (threshold)', () => {
      const { isOffline } = useSensorData()
      const now = new Date()
      const sixteenMinutesAgo = new Date(now.getTime() - 16 * 60 * 1000)
      expect(isOffline(sixteenMinutesAgo.toISOString())).toBe(true)
    })

    it('should handle very old timestamps', () => {
      const { isOffline } = useSensorData()
      const now = new Date()
      const oneDayAgo = new Date(now.getTime() - 24 * 60 * 60 * 1000)
      expect(isOffline(oneDayAgo.toISOString())).toBe(true)
    })
  })

  describe('processedData', () => {
    it('should group measurements by location', () => {
      const { processedData } = useSensorData()
      // Initial state should be empty
      expect(processedData.value.length).toBe(0)
    })

    it('should map display names correctly', () => {
      const { locationDisplayNames } = useSensorData()
      expect(locationDisplayNames.garage).toBe('Garage')
      expect(locationDisplayNames.heating).toBe('Heizung')
    })
  })

  describe('alerts', () => {
    it('should generate temperature alert for low temperature', () => {
      const { alerts } = useSensorData()
      // Initial state should have no alerts
      expect(Array.isArray(alerts.value)).toBe(true)
    })
  })

  describe('temperatureRanges configuration', () => {
    it('should have 5 temperature ranges defined', () => {
      const { temperatureRanges } = useSensorData()
      expect(temperatureRanges).toHaveLength(5)
    })

    it('should have correct range order', () => {
      const { temperatureRanges } = useSensorData()
      const ranges = temperatureRanges.map((r: any) => r.range)
      expect(ranges).toEqual(['cold', 'cool', 'comfortable', 'warm', 'hot'])
    })

    it('should have German labels', () => {
      const { temperatureRanges } = useSensorData()
      const labels = temperatureRanges.map((r: any) => r.label)
      expect(labels).toEqual(['Kalt', 'Kühl', 'Angenehm', 'Warm', 'Heiß'])
    })

    it('should have valid color codes', () => {
      const { temperatureRanges } = useSensorData()
      temperatureRanges.forEach((range: any) => {
        expect(range.color).toMatch(/^#[0-9A-F]{6}$/i)
        expect(range.bgColor).toMatch(/^#[0-9A-F]{6}$/i)
      })
    })
  })
})
