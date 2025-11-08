import { expect, test } from 'vitest'

test('dom exists', () => {
  console.log(typeof document)
  expect(typeof document).toBe('object')
})
