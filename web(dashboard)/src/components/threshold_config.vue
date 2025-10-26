<script setup lang="ts">
import { ref } from 'vue'
import { supabase } from '@/lib/supabase'
import type { ThresholdConfig } from '@/lib/supabase'
import '../style.css'

const props = defineProps<{
  thresholds: ThresholdConfig[]
}>()

const editingId = ref<number | null>(null)
const editForm = ref({
  threshold_value: 0,
  comparison_operator: '>' as '>' | '<' | '>=' | '<=' | '='
})

const isSaving = ref(false)
const message = ref<{ type: 'success' | 'error', text: string } | null>(null)

const startEdit = (threshold: ThresholdConfig) => {
  editingId.value = threshold.id
  editForm.value = {
    threshold_value: threshold.threshold_value,
    comparison_operator: threshold.comparison_operator
  }
}

const cancelEdit = () => {
  editingId.value = null
  editForm.value = {
    threshold_value: 0,
    comparison_operator: '>'
  }
}

const saveThreshold = async (thresholdId: number) => {
  isSaving.value = true
  message.value = null

  try {
    const { error } = await supabase
      .from('threshold_config')
      .update({
        threshold_value: editForm.value.threshold_value,
        comparison_operator: editForm.value.comparison_operator,
        updated_at: new Date().toISOString()
      })
      .eq('id', thresholdId)

    if (error) throw error

    message.value = { type: 'success', text: 'Threshold updated successfully!' }
    editingId.value = null
    
    // refresh page data
    setTimeout(() => {
      window.location.reload()
    }, 1000)

  } catch (error: any) {
    message.value = { type: 'error', text: error.message }
  } finally {
    isSaving.value = false
  }
}

const toggleActive = async (threshold: ThresholdConfig) => {
  try {
    const { error } = await supabase
      .from('threshold_config')
      .update({ is_active: !threshold.is_active })
      .eq('id', threshold.id)

    if (error) throw error

    message.value = { 
      type: 'success', 
      text: `Threshold ${threshold.is_active ? 'deactivated' : 'activated'}!` 
    }

    setTimeout(() => {
      window.location.reload()
    }, 1000)

  } catch (error: any) {
    message.value = { type: 'error', text: error.message }
  }
}

const operatorSymbols = {
  '>': 'Greater than (>)',
  '<': 'Less than (<)',
  '>=': 'Greater or equal (≥)',
  '<=': 'Less or equal (≤)',
  '=': 'Equal to (=)'
}
</script>

<template>
  <div class="card">
    <h2 class="card-title">Threshold Configuration</h2>

    <!-- message -->
    <div v-if="message" class="message" :class="message.type">
      {{ message.text }}
    </div>

    <div class="thresholds-list">
      <div 
        v-for="threshold in thresholds" 
        :key="threshold.id"
        class="threshold-item"
        :class="{ inactive: !threshold.is_active }"
      >
        <!-- view mode -->
        <div v-if="editingId !== threshold.id" class="threshold-view">
          <div class="threshold-header">
            <span class="sensor-label">{{ threshold.sensor_name.replace('_', ' ').toUpperCase() }}</span>
            <div class="actions">
              <button 
                @click="toggleActive(threshold)"
                class="btn-icon"
                :title="threshold.is_active ? 'Deactivate' : 'Activate'"
              >
                {{ threshold.is_active ? 'deactivate' : 'activate' }}
              </button>
              <button 
                @click="startEdit(threshold)"
                class="btn-icon"
                title="Edit"
              >
                ✏️
              </button>
            </div>
          </div>

          <div class="threshold-value-display">
            <span class="operator">{{ threshold.comparison_operator }}</span>
            <span class="value">{{ threshold.threshold_value }}</span>
          </div>

          <div class="threshold-description">
            {{ operatorSymbols[threshold.comparison_operator] }} {{ threshold.threshold_value }}
          </div>
        </div>

        <!-- edit mode -->
        <div v-else class="threshold-edit">
          <div class="threshold-header">
            <span class="sensor-label">{{ threshold.sensor_name.replace('_', ' ').toUpperCase() }}</span>
          </div>

          <div class="form-group">
            <label>Operator:</label>
            <select v-model="editForm.comparison_operator" class="form-select">
              <option value=">">Greater than (>)</option>
              <option value="<">Less than (<)</option>
              <option value=">=">Greater or equal (≥)</option>
              <option value="<=">Less or equal (≤)</option>
              <option value="=">Equal to (=)</option>
            </select>
          </div>

          <div class="form-group">
            <label>Threshold Value:</label>
            <input 
              v-model.number="editForm.threshold_value" 
              type="number" 
              step="0.01"
              class="form-input"
            />
          </div>

          <div class="edit-actions">
            <button 
              @click="saveThreshold(threshold.id)"
              :disabled="isSaving"
              class="btn-save"
            >
              {{ isSaving ? 'Saving...' : 'Save' }}
            </button>
            <button 
              @click="cancelEdit"
              :disabled="isSaving"
              class="btn-cancel"
            >
              Cancel
            </button>
          </div>
        </div>
      </div>
    </div>

    <div class="info-note">
      💡 The actuator stops when <strong>all active thresholds</strong> are met.
    </div>
  </div>
</template>

<style scoped>
.card {
  background: var(--bg-card);
  border-radius: 12px;
  padding: 1.5rem;
  box-shadow: var(--shadow);
  border: 1px solid var(--border-color);
}

.card-title {
  margin: 0 0 1.5rem 0;
  font-size: 1.25rem;
  font-weight: 600;
  color: var(--text-primary);
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.card-title::before {
  content: '⚙️';
  font-size: 1.5rem;
}

.message {
  padding: 0.75rem 1rem;
  border-radius: 8px;
  margin-bottom: 1rem;
  font-size: 0.9rem;
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

.thresholds-list {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.threshold-item {
  padding: 1rem;
  background: var(--bg-secondary);
  border-radius: 8px;
  border-left: 4px solid var(--accent-primary);
  transition: all 0.2s;
}

.threshold-item.inactive {
  opacity: 0.5;
  border-left-color: var(--text-muted);
}

.threshold-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 0.75rem;
}

.sensor-label {
  font-weight: 600;
  color: var(--accent-primary);
  font-size: 0.9rem;
  letter-spacing: 0.5px;
}

.actions {
  display: flex;
  gap: 0.5rem;
}

.btn-icon {
  background: transparent;
  border: none;
  font-size: 1.2rem;
  cursor: pointer;
  padding: 0.25rem 0.5rem;
  border-radius: 4px;
  transition: all 0.2s;
}

.btn-icon:hover {
  background: var(--bg-hover);
  transform: scale(1.1);
}

.threshold-value-display {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  margin-bottom: 0.5rem;
}

.operator {
  font-size: 1.5rem;
  color: var(--accent-primary);
  font-weight: 700;
}

.value {
  font-size: 2rem;
  font-weight: 700;
  color: var(--accent-light);
}

.threshold-description {
  font-size: 0.85rem;
  color: var(--text-muted);
}

/* Edit Mode Styles */
.form-group {
  margin-bottom: 1rem;
}

.form-group label {
  display: block;
  margin-bottom: 0.5rem;
  font-size: 0.9rem;
  color: var(--text-secondary);
  font-weight: 500;
}

.form-select,
.form-input {
  width: 100%;
  padding: 0.75rem;
  background: var(--bg-primary);
  border: 1px solid var(--border-color);
  border-radius: 6px;
  color: var(--text-primary);
  font-size: 0.95rem;
  transition: all 0.2s;
}

.form-select:focus,
.form-input:focus {
  outline: none;
  border-color: var(--accent-primary);
  box-shadow: 0 0 0 3px rgba(66, 184, 131, 0.1);
}

.edit-actions {
  display: flex;
  gap: 0.5rem;
  margin-top: 1rem;
}

.btn-save,
.btn-cancel {
  flex: 1;
  padding: 0.75rem;
  border: none;
  border-radius: 6px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.2s;
  font-size: 0.9rem;
}

.btn-save {
  background: var(--accent-primary);
  color: white;
}

.btn-save:hover:not(:disabled) {
  background: var(--accent-dark);
  transform: translateY(-1px);
}

.btn-save:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.btn-cancel {
  background: var(--bg-hover);
  color: var(--text-secondary);
  border: 1px solid var(--border-color);
}

.btn-cancel:hover:not(:disabled) {
  background: var(--bg-primary);
  color: var(--text-primary);
}

.info-note {
  margin-top: 1.5rem;
  padding: 1rem;
  background: var(--bg-hover);
  border-left: 3px solid var(--accent-primary);
  border-radius: 4px;
  font-size: 0.85rem;
  color: var(--text-secondary);
  line-height: 1.5;
}

.info-note strong {
  color: var(--accent-primary);
}
</style>