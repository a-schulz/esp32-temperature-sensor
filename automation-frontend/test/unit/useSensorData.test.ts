import { expect, test, vi } from 'vitest'

test('temperature range outside of definition', async () => {
  vi.stubEnv('VITE_SUPABASE_URL', 'https://abcd.supabase.co')
  vi.stubEnv('VITE_SUPABASE_ANON_KEY', '1234')
  const { useSensorData } = await import('../../src/composables/useSensorData')
  console.log('Env: ' + import.meta.env.VITE_SUPABASE_URL)
  expect(useSensorData().getTemperatureRange(99).range).toBe('comfortable')
})
