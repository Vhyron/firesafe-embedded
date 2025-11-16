<script setup lang="ts">
import { ref, computed } from 'vue'
import { supabase } from '@/lib/supabase'
import type { ThresholdConfig } from '@/lib/supabase'

const props = defineProps<{
  thresholds: ThresholdConfig[]
}>()

const emit = defineEmits<{
  updated: []
}>()

const editingId = ref<number | null>(null)
const editForm = ref({
  threshold_value: 0,
  comparison_operator: '>' as '>' | '<' | '>=' | '<=' | '='
})

const isSaving = ref(false)
const message = ref<{ type: 'success' | 'error', text: string } | null>(null)

// computed: active thresholds count
const activeCount = computed(() => 
  props.thresholds.filter(t => t.is_active).length
)

// computed: group thresholds by status
const activeThresholds = computed(() => 
  props.thresholds.filter(t => t.is_active)
)

const inactiveThresholds = computed(() => 
  props.thresholds.filter(t => !t.is_active)
)

const startEdit = (threshold: ThresholdConfig) => {
  editingId.value = threshold.id
  editForm.value = {
    threshold_value: threshold.threshold_value,
    comparison_operator: threshold.comparison_operator
  }
  message.value = null
}

const cancelEdit = () => {
  editingId.value = null
  editForm.value = {
    threshold_value: 0,
    comparison_operator: '>'
  }
  message.value = null
}

const saveThreshold = async (thresholdId: number) => {
  if (isNaN(editForm.value.threshold_value)) {
    message.value = { type: 'error', text: 'Please enter a valid number' }
    return
  }

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

    message.value = { type: 'success', text: 'Threshold updated successfully' }
    editingId.value = null
    
    emit('updated')
    
    setTimeout(() => {
      message.value = null
    }, 3000)

  } catch (error: any) {
    message.value = { type: 'error', text: error.message || 'Failed to update threshold' }
  } finally {
    isSaving.value = false
  }
}

const toggleActive = async (threshold: ThresholdConfig) => {
  try {
    const { error } = await supabase
      .from('threshold_config')
      .update({ 
        is_active: !threshold.is_active,
        updated_at: new Date().toISOString()
      })
      .eq('id', threshold.id)

    if (error) throw error

    message.value = { 
      type: 'success', 
      text: `${threshold.sensor_name} threshold ${threshold.is_active ? 'deactivated' : 'activated'}` 
    }

    emit('updated')

    setTimeout(() => {
      message.value = null
    }, 3000)

  } catch (error: any) {
    message.value = { type: 'error', text: error.message || 'Failed to toggle threshold' }
  }
}

const operatorSymbols = {
  '>': { symbol: '>', label: 'Greater than' },
  '<': { symbol: '<', label: 'Less than' },
  '>=': { symbol: '≥', label: 'Greater or equal' },
  '<=': { symbol: '≤', label: 'Less or equal' },
  '=': { symbol: '=', label: 'Equal to' }
}

const formatSensorName = (name: string) => {
  return name.replace('_', ' ').toUpperCase()
}

const formatDate = (dateString: string) => {
  const date = new Date(dateString)
  return date.toLocaleString('en-US', {
    month: 'short',
    day: 'numeric',
    hour: '2-digit',
    minute: '2-digit'
  })
}
</script>

<template>
  <div class="card">
    <div class="card-header">
      <div class="title-section">
        <h2 class="card-title">Threshold Configuration</h2>
        <div class="status-badge">
          {{ activeCount }} / {{ thresholds.length }} Active
        </div>
      </div>
    </div>

    <!-- global message -->
    <Transition name="slide-down">
      <div v-if="message" class="message" :class="message.type">
        <span class="message-icon">{{ message.type === 'success' ? '✓' : '!' }}</span>
        {{ message.text }}
      </div>
    </Transition>

    <!-- active thresholds -->
    <div v-if="activeThresholds.length > 0" class="section">
      <h3 class="section-title">Active</h3>
      <div class="thresholds-list">
        <TransitionGroup name="list">
          <div 
            v-for="threshold in activeThresholds" 
            :key="threshold.id"
            class="threshold-item active"
          >
            <!-- view mode -->
            <div v-if="editingId !== threshold.id" class="threshold-view">
              <div class="threshold-header">
                <div class="sensor-info">
                  <span class="sensor-label">{{ formatSensorName(threshold.sensor_name) }}</span>
                  <span class="last-updated">{{ formatDate(threshold.updated_at) }}</span>
                </div>
                <div class="actions">
                  <button 
                    @click="startEdit(threshold)"
                    class="btn-icon btn-edit"
                    title="Edit threshold"
                  >
                    ✏
                  </button>
                  <button 
                    @click="toggleActive(threshold)"
                    class="btn-icon btn-toggle"
                    title="Deactivate threshold"
                  >
                    ○
                  </button>
                </div>
              </div>

              <div class="threshold-display">
                <div class="operator-circle">
                  <span class="operator">{{ operatorSymbols[threshold.comparison_operator].symbol }}</span>
                </div>
                <div class="value-display">
                  <span class="value">{{ threshold.threshold_value.toFixed(2) }}</span>
                </div>
              </div>

              <div class="threshold-description">
                Triggers when {{ operatorSymbols[threshold.comparison_operator].label.toLowerCase() }} {{ threshold.threshold_value }}
              </div>
            </div>

            <!-- edit mode -->
            <div v-else class="threshold-edit">
              <div class="edit-header">
                <span class="sensor-label">{{ formatSensorName(threshold.sensor_name) }}</span>
                <span class="edit-badge">Editing</span>
              </div>

              <div class="form-grid">
                <div class="form-group">
                  <label class="form-label">Operator</label>
                  <select v-model="editForm.comparison_operator" class="form-select">
                    <option value=">">{{ operatorSymbols['>'].symbol }} Greater than</option>
                    <option value="<">{{ operatorSymbols['<'].symbol }} Less than</option>
                    <option value=">=">{{ operatorSymbols['>='].symbol }} Greater or equal</option>
                    <option value="<=">{{ operatorSymbols['<='].symbol }} Less or equal</option>
                    <option value="=">{{ operatorSymbols['='].symbol }} Equal to</option>
                  </select>
                </div>

                <div class="form-group">
                  <label class="form-label">Value</label>
                  <input 
                    v-model.number="editForm.threshold_value" 
                    type="number" 
                    step="0.01"
                    class="form-input"
                    placeholder="Enter value"
                  />
                </div>
              </div>

              <div class="preview-box">
                <strong>Preview:</strong> Trigger when {{ operatorSymbols[editForm.comparison_operator].label.toLowerCase() }} {{ editForm.threshold_value || 0 }}
              </div>

              <div class="edit-actions">
                <button 
                  @click="saveThreshold(threshold.id)"
                  :disabled="isSaving"
                  class="btn btn-save"
                >
                  {{ isSaving ? 'Saving...' : 'Save' }}
                </button>
                <button 
                  @click="cancelEdit"
                  :disabled="isSaving"
                  class="btn btn-cancel"
                >
                  Cancel
                </button>
              </div>
            </div>
          </div>
        </TransitionGroup>
      </div>
    </div>

    <!-- inactive thresholds -->
    <div v-if="inactiveThresholds.length > 0" class="section">
      <h3 class="section-title inactive-title">Inactive</h3>
      <div class="thresholds-list">
        <TransitionGroup name="list">
          <div 
            v-for="threshold in inactiveThresholds" 
            :key="threshold.id"
            class="threshold-item inactive"
          >
            <div class="threshold-view">
              <div class="threshold-header">
                <div class="sensor-info">
                  <span class="sensor-label">{{ formatSensorName(threshold.sensor_name) }}</span>
                  <span class="inactive-badge">Disabled</span>
                </div>
                <button 
                  @click="toggleActive(threshold)"
                  class="btn-icon btn-activate"
                  title="Activate threshold"
                >
                  ●
                </button>
              </div>

              <div class="threshold-display-compact">
                <span class="operator-text">{{ operatorSymbols[threshold.comparison_operator].symbol }}</span>
                <span class="value-text">{{ threshold.threshold_value }}</span>
              </div>
            </div>
          </div>
        </TransitionGroup>
      </div>
    </div>

    <!--info footer -->
    <div class="info-footer">
      <strong>Note:</strong> Actuator stops when all active thresholds are met
    </div>
  </div>
</template>

<style scoped>
  .card {
    background: var(--bg-card);
    border-radius: 10px;
    padding: 1rem;
    box-shadow: var(--shadow);
    border: 1px solid var(--border-color);
  }

  .card-header {
    margin-bottom: 1rem;
  }

  .title-section {
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 0.75rem;
  }

  .card-title {
    margin: 0;
    font-size: 1rem;
    font-weight: 600;
    color: var(--text-primary);
  }

  .status-badge {
    padding: 0.35rem 0.75rem;
    background: var(--success-bg);
    color: var(--success);
    border: 1px solid var(--success);
    border-radius: 12px;
    font-size: 0.75rem;
    font-weight: 600;
  }

  .message {
    padding: 0.65rem 0.85rem;
    border-radius: 6px;
    margin-bottom: 1rem;
    font-size: 0.8rem;
    display: flex;
    align-items: center;
    gap: 0.5rem;
    animation: slideIn 0.3s ease-out;
  }

  .message-icon {
    font-size: 1rem;
    flex-shrink: 0;
    font-weight: 700;
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

  .section {
    margin-bottom: 1.25rem;
  }

  .section:last-of-type {
    margin-bottom: 0;
  }

  .section-title {
    font-size: 0.75rem;
    font-weight: 600;
    color: var(--accent-primary);
    margin: 0 0 0.65rem 0;
    text-transform: uppercase;
    letter-spacing: 1px;
    display: flex;
    align-items: center;
    gap: 0.5rem;
  }

  .section-title::before {
    content: '';
    width: 3px;
    height: 12px;
    background: var(--accent-primary);
    border-radius: 2px;
  }

  .inactive-title {
    color: var(--text-muted);
  }

  .inactive-title::before {
    background: var(--text-muted);
  }

  .thresholds-list {
    display: flex;
    flex-direction: column;
    gap: 0.65rem;
  }

  .threshold-item {
    padding: 0.85rem;
    background: var(--bg-secondary);
    border-radius: 8px;
    transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    position: relative;
    overflow: hidden;
  }

  .threshold-item.active {
    border-left: 3px solid var(--accent-primary);
    box-shadow: 0 1px 4px rgba(66, 184, 131, 0.1);
  }

  .threshold-item.active:hover {
    transform: translateX(3px);
    box-shadow: 0 2px 8px rgba(66, 184, 131, 0.15);
  }

  .threshold-item.inactive {
    opacity: 0.6;
    border-left: 3px solid var(--text-muted);
  }

  .threshold-item.inactive:hover {
    opacity: 0.8;
  }

  .threshold-header {
    display: flex;
    justify-content: space-between;
    align-items: flex-start;
    margin-bottom: 0.65rem;
    gap: 0.75rem;
  }

  .sensor-info {
    display: flex;
    flex-direction: column;
    gap: 0.2rem;
  }

  .sensor-label {
    font-weight: 600;
    color: var(--accent-primary);
    font-size: 0.8rem;
    letter-spacing: 0.5px;
  }

  .last-updated {
    font-size: 0.7rem;
    color: var(--text-muted);
  }

  .inactive-badge {
    padding: 0.2rem 0.5rem;
    background: var(--bg-hover);
    color: var(--text-muted);
    border-radius: 10px;
    font-size: 0.7rem;
    font-weight: 600;
  }

  .actions {
    display: flex;
    gap: 0.4rem;
  }

  .btn-icon {
    background: var(--bg-hover);
    border: 1px solid var(--border-color);
    padding: 0.35rem 0.5rem;
    border-radius: 6px;
    cursor: pointer;
    transition: all 0.2s;
    font-size: 0.9rem;
    display: flex;
    align-items: center;
    justify-content: center;
    line-height: 1;
  }

  .btn-icon:hover {
    background: var(--bg-primary);
    transform: translateY(-1px);
    box-shadow: 0 2px 6px rgba(0, 0, 0, 0.2);
  }

  .btn-edit:hover {
    border-color: var(--accent-primary);
  }

  .btn-toggle:hover,
  .btn-activate:hover {
    border-color: var(--success);
  }

  .threshold-display {
    display: flex;
    align-items: center;
    gap: 1rem;
    margin-bottom: 0.65rem;
    padding: 0.75rem;
    background: linear-gradient(135deg, rgba(66, 184, 131, 0.05) 0%, transparent 100%);
    border-radius: 6px;
  }

  .operator-circle {
    width: 45px;
    height: 45px;
    border-radius: 50%;
    background: var(--accent-primary);
    display: flex;
    align-items: center;
    justify-content: center;
    box-shadow: 0 2px 8px rgba(66, 184, 131, 0.3);
    flex-shrink: 0;
  }

  .operator {
    font-size: 1.5rem;
    font-weight: 700;
    color: white;
    line-height: 1;
  }

  .value-display {
    display: flex;
    align-items: baseline;
    gap: 0.35rem;
  }

  .value {
    font-size: 1.75rem;
    font-weight: 700;
    color: var(--accent-light);
    line-height: 1;
  }

  .threshold-display-compact {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    padding: 0.5rem 0.75rem;
    background: var(--bg-hover);
    border-radius: 6px;
  }

  .operator-text {
    font-size: 1.25rem;
    color: var(--text-muted);
  }

  .value-text {
    font-size: 1.25rem;
    font-weight: 600;
    color: var(--text-secondary);
  }

  .threshold-description {
    font-size: 0.75rem;
    color: var(--text-secondary);
    padding: 0.5rem 0.75rem;
    background: var(--bg-hover);
    border-radius: 6px;
    line-height: 1.4;
  }

  .edit-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 0.85rem;
  }

  .edit-badge {
    padding: 0.25rem 0.65rem;
    background: linear-gradient(135deg, var(--accent-primary), var(--accent-dark));
    color: white;
    border-radius: 10px;
    font-size: 0.7rem;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 0.5px;
  }

  .form-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
    gap: 0.75rem;
    margin-bottom: 0.75rem;
  }

  .form-group {
    display: flex;
    flex-direction: column;
    gap: 0.35rem;
  }

  .form-label {
    font-size: 0.75rem;
    color: var(--text-secondary);
    font-weight: 600;
  }

  .form-select,
  .form-input {
    width: 100%;
    padding: 0.6rem 0.75rem;
    background: var(--bg-primary);
    border: 2px solid var(--border-color);
    border-radius: 6px;
    color: var(--text-primary);
    font-size: 0.8rem;
    transition: all 0.2s;
  }

  .form-select:focus,
  .form-input:focus {
    outline: none;
    border-color: var(--accent-primary);
    box-shadow: 0 0 0 3px rgba(66, 184, 131, 0.1);
    background: var(--bg-secondary);
  }

  .form-input::placeholder {
    color: var(--text-muted);
  }

  .preview-box {
    padding: 0.6rem 0.75rem;
    background: linear-gradient(135deg, rgba(66, 184, 131, 0.08) 0%, rgba(66, 184, 131, 0.02) 100%);
    border: 1px solid rgba(66, 184, 131, 0.2);
    border-radius: 6px;
    font-size: 0.75rem;
    color: var(--text-secondary);
    margin-bottom: 0.75rem;
    line-height: 1.4;
  }

  .preview-box strong {
    color: var(--accent-primary);
  }

  .edit-actions {
    display: flex;
    gap: 0.5rem;
  }

  .btn {
    flex: 1;
    padding: 0.6rem 0.85rem;
    border: none;
    border-radius: 6px;
    font-weight: 600;
    cursor: pointer;
    transition: all 0.2s;
    font-size: 0.8rem;
  }

  .btn-save {
    background: linear-gradient(135deg, var(--accent-primary), var(--accent-dark));
    color: white;
  }

  .btn-save:hover:not(:disabled) {
    transform: translateY(-1px);
    box-shadow: 0 3px 10px rgba(66, 184, 131, 0.3);
  }

  .btn-save:disabled {
    opacity: 0.6;
    cursor: not-allowed;
    transform: none;
  }

  .btn-cancel {
    background: var(--bg-hover);
    color: var(--text-secondary);
    border: 2px solid var(--border-color);
  }

  .btn-cancel:hover:not(:disabled) {
    background: var(--bg-primary);
    color: var(--text-primary);
    border-color: var(--accent-primary);
  }

  .info-footer {
    margin-top: 1rem;
    padding: 0.75rem;
    background: linear-gradient(135deg, rgba(66, 184, 131, 0.08) 0%, rgba(66, 184, 131, 0.02) 100%);
    border-left: 3px solid var(--accent-primary);
    border-radius: 6px;
    font-size: 0.75rem;
    color: var(--text-secondary);
    line-height: 1.5;
  }

  .info-footer strong {
    color: var(--accent-primary);
    font-weight: 600;
  }

  @keyframes slideIn {
    from {
      opacity: 0;
      transform: translateY(-10px);
    }
    to {
      opacity: 1;
      transform: translateY(0);
    }
  }

  .slide-down-enter-active,
  .slide-down-leave-active {
    transition: all 0.3s ease;
  }

  .slide-down-enter-from {
    opacity: 0;
    transform: translateY(-10px);
  }

  .slide-down-leave-to {
    opacity: 0;
    transform: translateY(-10px);
  }

  .list-enter-active,
  .list-leave-active {
    transition: all 0.3s ease;
  }

  .list-enter-from {
    opacity: 0;
    transform: translateX(-15px);
  }

  .list-leave-to {
    opacity: 0;
    transform: translateX(15px);
  }

  .list-move {
    transition: transform 0.3s ease;
  }

  @media (max-width: 768px) {
    .form-grid {
      grid-template-columns: 1fr;
    }

    .threshold-display {
      gap: 0.75rem;
    }

    .edit-actions {
      flex-direction: column;
    }
  }
</style>