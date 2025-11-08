#!/usr/bin/env node

import { existsSync, mkdirSync, writeFileSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'

const __dirname = dirname(fileURLToPath(import.meta.url))
const iconsDir = join(__dirname, '../public/icons')

// Ensure icons directory exists
if (!existsSync(iconsDir)) {
  mkdirSync(iconsDir, { recursive: true })
}

// SVG template for the temperature sensor icon
function createSVGIcon (size, color = '#1976D2') {
  return `
<svg width="${size}" height="${size}" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
  <rect width="24" height="24" fill="white"/>
  <path fill="${color}" d="M15 13V5a3 3 0 0 0-6 0v8a5 5 0 1 0 6 0M12 4a1 1 0 0 1 1 1v8.3a3 3 0 1 1-2 0V5a1 1 0 0 1 1-1"/>
  <circle cx="12" cy="17" r="1" fill="${color}"/>
  <path fill="${color}" d="M13 6h1v1h-1zm0 2h1v1h-1zm0 2h1v1h-1z"/>
</svg>`
}

// PNG-like SVG with solid background for better visibility
function createPNGLikeSVG (size) {
  return `
<svg width="${size}" height="${size}" viewBox="0 0 ${size} ${size}" xmlns="http://www.w3.org/2000/svg">
  <rect width="${size}" height="${size}" fill="#1976D2" rx="${size * 0.1}"/>
  <g transform="translate(${size * 0.2}, ${size * 0.2}) scale(${(size / 24) * 0.6})">
    <path fill="white" d="M15 13V5a3 3 0 0 0-6 0v8a5 5 0 1 0 6 0M12 4a1 1 0 0 1 1 1v8.3a3 3 0 1 1-2 0V5a1 1 0 0 1 1-1"/>
    <circle cx="12" cy="17" r="1" fill="white"/>
    <path fill="white" d="M13 6h1v1h-1zm0 2h1v1h-1zm0 2h1v1h-1z"/>
  </g>
</svg>`
}

// Apple touch icon with rounded corners
function createAppleTouchIcon () {
  return `
<svg width="180" height="180" viewBox="0 0 180 180" xmlns="http://www.w3.org/2000/svg">
  <rect width="180" height="180" fill="#1976D2" rx="36"/>
  <g transform="translate(45, 45) scale(3.75)">
    <path fill="white" d="M15 13V5a3 3 0 0 0-6 0v8a5 5 0 1 0 6 0M12 4a1 1 0 0 1 1 1v8.3a3 3 0 1 1-2 0V5a1 1 0 0 1 1-1"/>
    <circle cx="12" cy="17" r="1" fill="white"/>
    <path fill="white" d="M13 6h1v1h-1zm0 2h1v1h-1zm0 2h1v1h-1z"/>
  </g>
</svg>`
}

// Generate icons
const icons = [
  { name: 'pwa-192x192.png', size: 192, content: createPNGLikeSVG(192) },
  { name: 'pwa-512x512.png', size: 512, content: createPNGLikeSVG(512) },
  { name: 'pwa-144x144.png', size: 144, content: createPNGLikeSVG(144) },
  {
    name: 'apple-touch-icon.png',
    size: 180,
    content: createAppleTouchIcon(),
  },
  { name: 'mask-icon.svg', size: 24, content: createSVGIcon(24, '#000000') },
]

// Write icon files (as SVG with .png extension for simplicity)
for (const icon of icons) {
  const filePath = join(iconsDir, icon.name)
  writeFileSync(filePath, icon.content)
  console.log(`Generated: ${icon.name}`)
}

console.log('PWA icons generated successfully!')
console.log('Note: These are SVG files with PNG extensions for development.')
console.log(
  'For production, consider using proper PNG files generated from the SVGs.',
)
