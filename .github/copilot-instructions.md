---
description: 'VueJS 3 development standards and best practices with Composition API and TypeScript'
applyTo: '**/*.vue, **/*.ts, **/*.js, **/*.scss'
---

# VueJS 3 Development Instructions

Instructions for building high-quality VueJS 3 applications with the Composition API, TypeScript, and modern best practices.

## Project Context

- Vue 3.5.x with Composition API as default
- TypeScript 5.8.x for type safety
- Single File Components (`.vue`) with `<script setup>` syntax
- Vite 6.3.x as build tool
- Pinia 3.0.x for application state management
- Vue Router 4.5.x with auto-routing
- Vuetify 3.9.x as component library
- TailwindCSS 4.1.x for utility-first styling
- Official Vue style guide and best practices

## Development Standards

### Architecture
- Favor the Composition API (`setup` functions and composables) over the Options API
- Organize components and composables by feature or domain for scalability
- Separate UI-focused components (presentational) from logic-focused components (containers)
- Extract reusable logic into composable functions in a `composables/` directory
- Structure store modules (Pinia) by domain, with clearly defined actions, state, and getters in the `stores/` directory
- Use Vue Layouts plugin for consistent page layouts across the application
- Implement auto-imports for Vue, Vue Router, and Pinia functions to reduce boilerplate

### TypeScript Integration
- Enable `strict` mode in `tsconfig.json` for maximum type safety
- Use `defineComponent` or `<script setup lang="ts">` with `defineProps` and `defineEmits`
- Leverage `PropType<T>` for typed props and default values
- Use type aliases for complex prop and state shapes
- Define types for event handlers, refs, and `useRoute`/`useRouter` hooks
- Implement generic components and composables where applicable also make use of generic types
- Utilize Vue's type inference for reactive state with `ref<T>` and `reactive<T>`
- Follow the @vue/tsconfig base configuration for consistent TypeScript settings

### Component Design
- Adhere to the single responsibility principle for components
- Use PascalCase for component names
- Keep components small and focused on one concern
- Use `<script setup lang="ts">` syntax for brevity and performance
- Validate props with TypeScript; use runtime checks only when necessary
- Favor slots and scoped slots for flexible composition
- Use auto-registered components with unplugin-vue-components
- Implement component interfaces with careful consideration of prop types and events

### State Management
- Use Pinia for global state: define stores with `defineStore`
- For simple local state, use `ref` within `setup`
- Use `computed` for derived state
- Keep state normalized for complex structures
- Use actions in Pinia stores for asynchronous logic
- Implement Vue3-Toastify for async operations and user feedback
- Leverage store plugins for persistence or debugging
- Use VueUse's utilities for local storage state persistence

### Composition API Patterns
- Create reusable composables for shared logic
- Use `watch` and `watchEffect` with precise dependency lists
- Cleanup side effects in `onUnmounted` or `watch` cleanup callbacks
- Use `defineExpose` and `defineEmits` for component APIs
- Leverage VueUse's composables for common tasks (mouse tracking, network status, etc.)
- Implement memoization for expensive computations using lodash or VueUse

### Styling
- Use `<style scoped>` for component-level styles or CSS Modules
- Use Vuetify as component library, if adjustments are needed create new base components wrapping the vuetify component
  in the `components/base/` directory
- Implement TailwindCSS for utility-first styling alongside Vuetify
- Follow BEM or functional CSS conventions for class naming when custom CSS is needed
- Leverage CSS custom properties for theming and design tokens
- Implement mobile-first, responsive design with CSS Flex/Grid
- Ensure styles are accessible (contrast, focus states)
- Use predefined colors for the themes defined in `plugins/vuetify.ts`
- Use PostCSS with SCSS for advanced styling needs

### Performance Optimization
- Avoid unnecessary watchers; prefer `computed` where possible
- Tree-shake unused code and leverage Vite's optimization features
- Use localStorage with cache invalidation strategies for caching
- Implement dynamic imports for route components to enable code-splitting
- Use Suspense for asynchronous components
- Optimize Vuetify imports with vite-plugin-vuetify
- Implement proper memoization for expensive computations
- Use skeleton loaders for better perceived performance

### Data Fetching

- For data fetching mainly use the api clients generated from OpenAPI specs located in `clients/` directory
- Leverage @hey-api/client-fetch and @hey-api/openapi-ts for type-safe API consumption
- Use `async/await` syntax for clarity
- Encapsulate API calls in composables or Pinia actions
- Handle loading, error, and success states explicitly
- Cancel stale requests on component unmount or param change
- Implement optimistic updates with rollbacks on failure
- Cache responses and use background revalidation
- Use VueUse's useFetch or similar utilities for simple requests

### Error Handling
- Wrap risky logic in `try/catch`; provide user-friendly messages
- Implement global error handling with Vue's errorCaptured hook
- Use vue3-toastify for error notifications
- Create an error boundary component for catching and displaying errors
- Log errors to monitoring services when in production
- Provide fallback UI states for failed API requests

### Forms and Validation

- Use VeeValidate 4.x with Yup for declarative form validation
- Build forms with controlled `v-model` bindings
- Validate on blur or input with debouncing for performance
- Handle file uploads and complex multi-step forms in composables
- Ensure accessible labeling, error announcements, and focus management
- Use VuePic DatePicker for date/time inputs
- Consider form state persistence for multi-step forms

### Analytics and User Tracking
- Integrate Umami as analytics solution
- Track page views and custom events via router hooks and composables
- Create dedicated analytics composables for reusable tracking logic
- Set up custom event tracking for important user interactions

### Routing

- Use Vue Router 4.x with unplugin-vue-router for automatic route generation based on file structure
- Use `useRoute` and `useRouter` in `setup` for programmatic navigation
- Manage query params and dynamic segments properly with type safety
- Implement route meta for page titles, authentication requirements, and layout selection
- Use vite-plugin-vue-layouts for consistent page layouts
- Implement navigation guards for auth protection and data prefetching
- Handle route transitions with animation for better UX

### Testing
- Write unit tests using packages like `vitest`, `happy-dom` and `@testing-library/vue`
- Focus on behavior, not implementation details
- Use `mount` and `shallowMount` for component isolation
- Mock global plugins (router, Pinia) as needed
- Add end-to-end tests with Cypress
- Test accessibility using axe-core integration
- Implement snapshot testing for UI components
- Create testing utilities for common testing patterns

### Security
- Avoid using `v-html`; sanitize any HTML inputs rigorously
- Use CSP headers to mitigate XSS and injection attacks
- Validate and escape data in templates and directives
- Use HTTPS for all API requests
- Store sensitive tokens in HTTP-only cookies, not `localStorage`
- Implement proper CORS configuration
- Follow OWASP security guidelines for frontend applications
- Sanitize user input before sending to APIs

### Accessibility
- Use semantic HTML elements and ARIA attributes
- Manage focus for modals and dynamic content
- Provide keyboard navigation for interactive components
- Add meaningful `alt` text for images and icons
- Ensure color contrast meets WCAG AA standards
- Test with screen readers and keyboard-only navigation
- Implement skip links for keyboard users
- Use Vuetify's built-in accessibility features

### Richtext Editor Integration

- Utilize TipTap for rich text editing (version 3.0.x)
- Configure extension-bubble-menu and extension-placeholder for enhanced editing
- Create composables to encapsulate editor state and logic
- Implement custom extensions for domain-specific formatting needs
- Provide clear validation for editor content

### Modal & Dialog Management

- Use Vue Final Modal (v4.5.x) for modals and dialogs
- Create wrapper components for common modal patterns
- Manage modal state with composables or Pinia
- Ensure proper focus management and keyboard accessibility
- Implement stacking for multiple open modals
- Provide consistent animation and styling

### Internationalization

- Implement Vue-i18n (v10.0.x) for internationalization
- Organize translations by feature domain
- Support plural forms and date/number formatting
- Implement locale switching with persistent preference
- Consider lazy-loading translations for less common languages
- Extract translations with proper context for translators

### Data Visualization

- Use Chart.js (v4.3.x) with vue-chartjs (v5.1.x) for charts and graphs
- Create reusable chart components with props for configuration
- Handle dynamic data updates with reactive props
- Ensure charts are accessible with proper labeling and descriptions
- Optimize performance for large datasets with virtualization techniques

## Implementation Process
1. Plan component and composable architecture
2. Initialize Vite project with Vue 3 and TypeScript
3. Define Pinia stores and composables
4. Create core UI components and layout
5. Integrate routing and navigation
6. Implement data fetching and state logic
7. Build forms with validation and error states
8. Add global error handling and fallback UIs
9. Add unit and E2E tests
10. Optimize performance and bundle size
11. Ensure accessibility compliance
12. Document components, composables, and stores

## Additional Guidelines

- Follow Vue's official style guide (vuejs.org/style-guide)
- Use ESLint (with `plugin:vue/vue3-recommended`) and Prettier for code consistency
- Write meaningful commit messages and maintain clean git history
- Keep dependencies up to date and audit for vulnerabilities
- Document complex logic with JSDoc/TSDoc
- Use Vue DevTools for debugging and profiling
- Leverage WebStorm's and VSCode's Vue.js support for improved development experience
- Use npm for package management following the project setup

## Common Patterns
- Renderless components and scoped slots for flexible UI
- Custom directives for cross-cutting concerns
- Use VueFinalModal for modals and dialogs
- Plugin system for global utilities (i18n, etc.)
- Composable factories for parameterized logic
- Loading overlays with vue-loading-overlay
- TailwindCSS for utility classes alongside Vuetify components
- Auto-imported components with unplugin-vue-components
