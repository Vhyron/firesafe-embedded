<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { supabase } from '@/lib/supabase'

interface ServoConfig {
  id: number
  mode: 'automatic' | 'manual'
  manual_position: number | null
  updated_at: string
}

const mode = ref<'automatic' | 'manual'>('automatic')
const currentPosition = ref(90)
const loading = ref(false)
const message = ref<{ type: 'success' | 'error', text: string } | null>(null)

// position buttons
const positions = [0, 30, 60, 90, 120, 150, 180]

// fetch current mode
const fetchCurrentMode = async () => {
  try {
    const { data, error } = await supabase
      .from('servo_config')
      .select('*')
      .eq('id', 1)
      .single()

    if (error) throw error
    
    if (data) {
      mode.value = data.mode
      currentPosition.value = data.manual_position ?? 90
    }
  } catch (error) {
    console.error('Error fetching servo config:', error)
  }
}

// switch mode (automatic/manual)
const switchMode = async (newMode: 'automatic' | 'manual') => {
  loading.value = true
  message.value = null

  try {
    const SUPABASE_URL = import.meta.env.VITE_SUPABASE_URL
    const SUPABASE_ANON_KEY = import.meta.env.VITE_SUPABASE_ANON_KEY

    const payload = {
      mode: newMode,
      ...(newMode === 'manual' && { manual_position: currentPosition.value }),
    }

    const response = await fetch(`${SUPABASE_URL}/functions/v1/set-servo-mode`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${SUPABASE_ANON_KEY}`,
      },
      body: JSON.stringify(payload),
    })

    const data = await response.json()

    if (data.success) {
      mode.value = newMode
      message.value = { 
        type: 'success', 
        text: `✓ Switched to ${newMode} mode` 
      }
      setTimeout(() => message.value = null, 3000)
    } else {
      message.value = { 
        type: 'error', 
        text: `✗ Error: ${data.error}` 
      }
    }
  } catch (error: any) {
    message.value = { 
      type: 'error', 
      text: `✗ Error: ${error.message}` 
    }
  } finally {
    loading.value = false
  }
}

// set manual position
const setManualPosition = async (position: number) => {
  loading.value = true
  message.value = null

  try {
    const SUPABASE_URL = import.meta.env.VITE_SUPABASE_URL
    const SUPABASE_ANON_KEY = import.meta.env.VITE_SUPABASE_ANON_KEY

    const response = await fetch(`${SUPABASE_URL}/functions/v1/set-servo-mode`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${SUPABASE_ANON_KEY}`,
      },
      body: JSON.stringify({
        mode: 'manual',
        manual_position: position,
      }),
    })

    const data = await response.json()

    if (data.success) {
      currentPosition.value = position
      message.value = { 
        type: 'success', 
        text: `✓ Moved to ${position}°` 
      }
      setTimeout(() => message.value = null, 3000)
    } else {
      message.value = { 
        type: 'error', 
        text: `✗ Error: ${data.error}` 
      }
    }
  } catch (error: any) {
    message.value = { 
      type: 'error', 
      text: `✗ Error: ${error.message}` 
    }
  } finally {
    loading.value = false
  }
}

let refreshInterval: any

onMounted(() => {
  fetchCurrentMode()
  // poll every 5 seconds
  refreshInterval = setInterval(fetchCurrentMode, 3000)
})

onUnmounted(() => {
  if (refreshInterval) clearInterval(refreshInterval)
})
</script>

<template>
  <div class="card">
    <div class="card-header">
      <h2 class="card-title">Servo Control</h2>
      <div class="mode-badge" :class="mode">
        {{ mode === 'automatic' ? '🤖 Auto' : '🎮 Manual' }}
      </div>
    </div>

    <!-- status message -->
    <Transition name="slide-down">
      <div v-if="message" class="message" :class="message.type">
        {{ message.text }}
      </div>
    </Transition>

    <!-- mode toggle -->
    <div class="mode-toggle">
      <button
        @click="switchMode('automatic')"
        :disabled="loading || mode === 'automatic'"
        class="mode-button"
        :class="{ active: mode === 'automatic' }"
      >
        🤖 Automatic
      </button>
      <button
        @click="switchMode('manual')"
        :disabled="loading || mode === 'manual'"
        class="mode-button"
        :class="{ active: mode === 'manual' }"
      >
        🎮 Manual
      </button>
    </div>

    <!-- mode description -->
    <div class="mode-description">
      <p v-if="mode === 'automatic'">
        Servo moves automatically based on sensor thresholds (0° ↔ 180° pattern)
      </p>
      <p v-else>
        Manual control enabled - select position below
      </p>
    </div>

    <!-- manual controls (only show in manual mode) -->
    <div v-if="mode === 'manual'" class="manual-controls">
      <!-- current position display -->
      <div class="position-display">
        <div class="position-value">{{ currentPosition }}°</div>
        <div class="position-label">Current Position</div>
      </div>

      <!-- position slider visual -->
      <div class="position-slider">
        <div class="slider-track">
          <div 
            class="slider-fill" 
            :style="{ width: `${(currentPosition / 180) * 100}%` }"
          />
          <div 
            class="slider-thumb"
            :style="{ left: `${(currentPosition / 180) * 100}%` }"
          />
        </div>
        <div class="slider-labels">
          <span>0°</span>
          <span>90°</span>
          <span>180°</span>
        </div>
      </div>

      <!-- position buttons grid -->
      <div class="position-grid">
        <button
          v-for="pos in positions"
          :key="pos"
          @click="setManualPosition(pos)"
          :disabled="loading || currentPosition === pos"
          class="position-button"
          :class="{ active: currentPosition === pos }"
        >
          {{ pos }}°
        </button>
      </div>

      <!-- quick actions -->
      <div class="quick-actions">
        <button
          @click="setManualPosition(0)"
          :disabled="loading"
          class="quick-button min"
        >
          ← Min (0°)
        </button>
        <button
          @click="setManualPosition(90)"
          :disabled="loading"
          class="quick-button center"
        >
          Center (90°)
        </button>
        <button
          @click="setManualPosition(180)"
          :disabled="loading"
          class="quick-button max"
        >
          Max (180°) →
        </button>
      </div>
    </div>

    <!-- info footer -->
    <div class="info-footer">
      <div class="info-item">
        <strong>Automatic:</strong> Sensor-driven, 30° steps, 3s intervals
      </div>
      <div class="info-item">
        <strong>Manual:</strong> Direct control, instant positioning
      </div>
    </div>
  </div>
</template>

<style scoped>
.card {
  background: var(--bg-card);
  border-radius: 8px;
  padding: 0.65rem;
  box-shadow: var(--shadow);
  border: 1px solid var(--border-color);
  height: 100%;
  display: flex;
  flex-direction: column;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 0.6rem;
}

.card-title {
  margin: 0;
  font-size: 0.8rem;
  font-weight: 600;
  color: var(--text-primary);
}

.mode-badge {
  padding: 0.25rem 0.5rem;
  border-radius: 10px;
  font-size: 0.65rem;
  font-weight: 600;
  transition: all 0.3s;
}

.mode-badge.automatic {
  background: var(--success-bg);
  color: var(--success);
  border: 1px solid var(--success);
}

.mode-badge.manual {
  background: var(--bg-hover);
  color: var(--accent-primary);
  border: 1px solid var(--accent-primary);
}

.message {
  padding: 0.5rem 0.65rem;
  border-radius: 6px;
  margin-bottom: 0.65rem;
  font-size: 0.75rem;
  animation: slideIn 0.3s ease-out;
}

.message.success {
  background: var(--success-bg);
  color: var(--success);
  border: 1px solid var(--success);
}

.message.error {
  background: var(--error-bg);
  color: var(--error);
  border: 1px solid var(--error);
}

.mode-toggle {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 0.5rem;
  margin-bottom: 0.65rem;
}

.mode-button {
  padding: 0.65rem;
  background: var(--bg-secondary);
  color: var(--text-secondary);
  border: 2px solid var(--border-color);
  border-radius: 6px;
  cursor: pointer;
  font-weight: 600;
  font-size: 0.8rem;
  transition: all 0.2s;
}

.mode-button:hover:not(:disabled) {
  background: var(--bg-hover);
  color: var(--text-primary);
  transform: translateY(-1px);
}

.mode-button.active {
  background: var(--accent-primary);
  color: white;
  border-color: var(--accent-primary);
  transform: scale(1.01);
}

.mode-button:disabled {
  opacity: 0.6;
  cursor: not-allowed;
  transform: none;
}

.mode-description {
  padding: 0.65rem;
  background: var(--bg-secondary);
  border-radius: 6px;
  margin-bottom: 0.75rem;
}

.mode-description p {
  margin: 0;
  font-size: 0.75rem;
  color: var(--text-secondary);
  line-height: 1.4;
}

.manual-controls {
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
}

.position-display {
  text-align: center;
  padding: 0.85rem;
  background: linear-gradient(135deg, rgba(66, 184, 131, 0.1) 0%, transparent 100%);
  border-radius: 8px;
  border: 2px solid var(--accent-primary);
}

.position-value {
  font-size: 2.25rem;
  font-weight: 700;
  color: var(--accent-primary);
  line-height: 1;
  margin-bottom: 0.3rem;
}

.position-label {
  font-size: 0.7rem;
  color: var(--text-muted);
  font-weight: 500;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.position-slider {
  display: flex;
  flex-direction: column;
  gap: 0.35rem;
}

.slider-track {
  position: relative;
  height: 10px;
  background: var(--bg-secondary);
  border-radius: 5px;
  overflow: hidden;
}

.slider-fill {
  position: absolute;
  left: 0;
  top: 0;
  height: 100%;
  background: linear-gradient(90deg, var(--accent-primary), var(--accent-light));
  transition: width 0.5s cubic-bezier(0.4, 0, 0.2, 1);
}

.slider-thumb {
  position: absolute;
  top: 50%;
  width: 20px;
  height: 20px;
  background: white;
  border: 2px solid var(--accent-primary);
  border-radius: 50%;
  transform: translate(-50%, -50%);
  box-shadow: 0 2px 6px rgba(0, 0, 0, 0.2);
  transition: left 0.5s cubic-bezier(0.4, 0, 0.2, 1);
}

.slider-labels {
  display: flex;
  justify-content: space-between;
  font-size: 0.65rem;
  color: var(--text-muted);
  padding: 0 0.2rem;
}

.position-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 0.5rem;
}

.position-button {
  padding: 0.65rem;
  background: var(--bg-secondary);
  color: var(--text-primary);
  border: 2px solid var(--border-color);
  border-radius: 6px;
  cursor: pointer;
  font-weight: 700;
  font-size: 0.85rem;
  transition: all 0.2s;
}

.position-button:hover:not(:disabled) {
  background: var(--bg-hover);
  border-color: var(--accent-primary);
  transform: translateY(-1px);
}

.position-button.active {
  background: var(--accent-primary);
  color: white;
  border-color: var(--accent-primary);
  transform: scale(1.03);
}

.position-button:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}

.quick-actions {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 0.5rem;
}

.quick-button {
  padding: 0.6rem;
  color: white;
  border: none;
  border-radius: 6px;
  cursor: pointer;
  font-weight: 600;
  font-size: 0.7rem;
  transition: all 0.2s;
}

.quick-button:hover:not(:disabled) {
  transform: translateY(-1px);
  box-shadow: 0 3px 8px rgba(0, 0, 0, 0.2);
}

.quick-button:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}

.quick-button.min {
  background: linear-gradient(135deg, #667eea, #764ba2);
}

.quick-button.center {
  background: linear-gradient(135deg, #42b883, #35495e);
}

.quick-button.max {
  background: linear-gradient(135deg, #f093fb, #f5576c);
}

.info-footer {
  margin-top: 0.75rem;
  padding: 0.65rem;
  background: var(--bg-secondary);
  border-radius: 6px;
  display: flex;
  flex-direction: column;
  gap: 0.35rem;
}

.info-item {
  font-size: 0.7rem;
  color: var(--text-secondary);
  line-height: 1.4;
}

.info-item strong {
  color: var(--accent-primary);
  font-weight: 600;
}

@keyframes slideIn {
  from { opacity: 0; transform: translateY(-10px); }
  to { opacity: 1; transform: translateY(0); }
}

@media (max-width: 768px) {
  .position-grid {
    grid-template-columns: repeat(3, 1fr);
  }
  .quick-actions {
    grid-template-columns: 1fr;
  }
  .position-value {
    font-size: 1.85rem;
  }
}
</style>