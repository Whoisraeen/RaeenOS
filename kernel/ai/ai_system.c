#include "ai_system.h"

// Global AI system
static ai_system_t ai_system;

// Simple neural network simulation
static float* simulate_neural_network(float* input, uint32_t input_size, uint32_t output_size) {
    float* output = kmalloc(output_size * sizeof(float));
    if (!output) {
        return NULL;
    }
    
    // Simple linear transformation simulation
    for (uint32_t i = 0; i < output_size; i++) {
        output[i] = 0.0f;
        for (uint32_t j = 0; j < input_size; j++) {
            output[i] += input[j] * 0.1f;  // Simple weight simulation
        }
        // Simple activation function (sigmoid-like)
        output[i] = output[i] / (1.0f + output[i] * output[i]);
    }
    
    return output;
}

// Simple training simulation
static void simulate_training(ai_model_t* model, ai_dataset_t* dataset) {
    if (!model || !dataset) {
        return;
    }
    
    model->state = AI_MODEL_STATE_TRAINING;
    uint64_t start_time = get_system_time();
    
    // Simulate training epochs
    for (uint32_t epoch = 0; epoch < model->epochs; epoch++) {
        // Simulate batch processing
        for (uint32_t batch = 0; batch < dataset->samples / model->batch_size; batch++) {
            // Simulate forward pass
            float loss = 0.0f;
            for (uint32_t sample = 0; sample < model->batch_size; sample++) {
                // Simulate loss calculation
                loss += 0.1f * (1.0f - (float)sample / model->batch_size);
            }
            loss /= model->batch_size;
            
            // Update model loss
            model->loss = loss;
            
            // Simulate validation
            if (epoch % 10 == 0) {
                model->validation_loss = loss * 1.1f;  // Slightly higher validation loss
            }
        }
        
        // Simulate accuracy improvement
        model->accuracy = 0.5f + (float)epoch / model->epochs * 0.4f;  // 50% to 90%
        model->precision = model->accuracy * 0.95f;
        model->recall = model->accuracy * 0.92f;
        model->f1_score = 2.0f * (model->precision * model->recall) / (model->precision + model->recall);
    }
    
    model->training_time = get_system_time() - start_time;
    model->state = AI_MODEL_STATE_TRAINED;
    model->last_training = get_system_time();
    
    printf("Model training completed: accuracy=%.2f%%, loss=%.4f, time=%llums\n",
           model->accuracy * 100.0f, model->loss, model->training_time);
}

int ai_system_init(void) {
    spinlock_init(&ai_system.lock);
    ai_system.initialized = true;
    ai_system.model_count = 0;
    ai_system.active_model_count = 0;
    ai_system.dataset_count = 0;
    ai_system.task_count = 0;
    ai_system.running_task_count = 0;
    ai_system.pipeline_count = 0;
    ai_system.agent_count = 0;
    ai_system.api_count = 0;
    
    // Initialize system configuration
    ai_system.gpu_acceleration = false;
    ai_system.distributed_computing = false;
    ai_system.cloud_integration = false;
    ai_system.max_concurrent_tasks = 10;
    ai_system.max_memory_usage = 1024;  // 1GB default
    
    // Initialize performance metrics
    ai_system.cpu_usage = 0.0f;
    ai_system.memory_usage = 0.0f;
    ai_system.gpu_usage = 0.0f;
    ai_system.total_inference_time = 0;
    ai_system.total_training_time = 0;
    
    // Initialize arrays
    memset(ai_system.models, 0, sizeof(ai_system.models));
    memset(ai_system.active_models, 0, sizeof(ai_system.active_models));
    memset(ai_system.datasets, 0, sizeof(ai_system.datasets));
    memset(ai_system.tasks, 0, sizeof(ai_system.tasks));
    memset(ai_system.running_tasks, 0, sizeof(ai_system.running_tasks));
    memset(ai_system.pipelines, 0, sizeof(ai_system.pipelines));
    memset(ai_system.agents, 0, sizeof(ai_system.agents));
    memset(ai_system.apis, 0, sizeof(ai_system.apis));
    
    printf("AI system initialized\n");
    
    return 0;
}

void ai_system_shutdown(void) {
    spinlock_acquire(&ai_system.lock);
    
    // Clean up models
    for (uint32_t i = 0; i < ai_system.model_count; i++) {
        if (ai_system.models[i]) {
            ai_model_destroy(ai_system.models[i]);
        }
    }
    
    // Clean up datasets
    for (uint32_t i = 0; i < ai_system.dataset_count; i++) {
        if (ai_system.datasets[i]) {
            ai_dataset_destroy(ai_system.datasets[i]);
        }
    }
    
    // Clean up tasks
    for (uint32_t i = 0; i < ai_system.task_count; i++) {
        if (ai_system.tasks[i]) {
            ai_task_destroy(ai_system.tasks[i]);
        }
    }
    
    // Clean up pipelines
    for (uint32_t i = 0; i < ai_system.pipeline_count; i++) {
        if (ai_system.pipelines[i]) {
            ai_pipeline_destroy(ai_system.pipelines[i]);
        }
    }
    
    // Clean up agents
    for (uint32_t i = 0; i < ai_system.agent_count; i++) {
        if (ai_system.agents[i]) {
            ai_agent_destroy(ai_system.agents[i]);
        }
    }
    
    // Clean up APIs
    for (uint32_t i = 0; i < ai_system.api_count; i++) {
        if (ai_system.apis[i]) {
            ai_api_destroy(ai_system.apis[i]);
        }
    }
    
    ai_system.initialized = false;
    spinlock_release(&ai_system.lock);
}

ai_system_t* ai_system_get_system(void) {
    return &ai_system;
}

ai_model_t* ai_model_create(const char* name, ai_model_type_t type, ai_task_type_t task_type) {
    if (!ai_system.initialized || !name) {
        return NULL;
    }
    
    spinlock_acquire(&ai_system.lock);
    
    // Check if model already exists
    for (uint32_t i = 0; i < ai_system.model_count; i++) {
        if (ai_system.models[i] && strcmp(ai_system.models[i]->name, name) == 0) {
            spinlock_release(&ai_system.lock);
            return NULL;
        }
    }
    
    // Allocate model structure
    ai_model_t* model = kmalloc(sizeof(ai_model_t));
    if (!model) {
        spinlock_release(&ai_system.lock);
        return NULL;
    }
    
    // Initialize model
    memset(model, 0, sizeof(ai_model_t));
    model->id = ai_system.model_count + 1;
    strncpy(model->name, name, MAX_MODEL_NAME_LENGTH - 1);
    model->type = type;
    model->state = AI_MODEL_STATE_UNLOADED;
    model->task_type = task_type;
    model->model_data = NULL;
    model->model_size = 0;
    model->input_features = 10;  // Default
    model->output_features = 1;  // Default
    model->training_samples = 0;
    model->validation_samples = 0;
    model->test_samples = 0;
    model->accuracy = 0.0f;
    model->precision = 0.0f;
    model->recall = 0.0f;
    model->f1_score = 0.0f;
    model->loss = 1.0f;
    model->validation_loss = 1.0f;
    model->epochs = 100;  // Default
    model->batch_size = 32;  // Default
    model->learning_rate = 0.001f;  // Default
    model->training_time = 0;
    model->last_training = 0;
    model->gpu_accelerated = false;
    model->quantized = false;
    model->gpu_memory_usage = 0;
    strncpy(model->version, "1.0.0", 31);
    strncpy(model->author, "RaeenOS AI", 63);
    strncpy(model->description, "AI model created by RaeenOS", 255);
    model->created_time = get_system_time();
    model->modified_time = model->created_time;
    
    // Add to model list
    ai_system.models[ai_system.model_count++] = model;
    
    spinlock_release(&ai_system.lock);
    
    printf("Created AI model: %s (type: %d, task: %d)\n", name, type, task_type);
    
    return model;
}

int ai_model_train(ai_model_t* model, ai_dataset_t* dataset, void* parameters) {
    if (!model || !dataset) {
        return -1;
    }
    
    spinlock_acquire(&ai_system.lock);
    
    // Update model with dataset information
    model->training_samples = dataset->samples * dataset->train_split / 100;
    model->validation_samples = dataset->samples * dataset->validation_split / 100;
    model->test_samples = dataset->samples * dataset->test_split / 100;
    model->input_features = dataset->features;
    model->output_features = dataset->classes > 0 ? dataset->classes : 1;
    
    // Update model state
    model->state = AI_MODEL_STATE_TRAINING;
    model->modified_time = get_system_time();
    
    spinlock_release(&ai_system.lock);
    
    // Simulate training (this would be async in a real implementation)
    simulate_training(model, dataset);
    
    return 0;
}

int ai_model_inference(ai_model_t* model, void* input, void* output) {
    if (!model || !input || !output) {
        return -1;
    }
    
    if (model->state != AI_MODEL_STATE_TRAINED && model->state != AI_MODEL_STATE_INFERENCE) {
        return -1;
    }
    
    spinlock_acquire(&ai_system.lock);
    
    model->state = AI_MODEL_STATE_INFERENCE;
    uint64_t start_time = get_system_time();
    
    // Simulate inference
    float* input_data = (float*)input;
    float* output_data = (float*)output;
    
    float* result = simulate_neural_network(input_data, model->input_features, model->output_features);
    if (result) {
        memcpy(output_data, result, model->output_features * sizeof(float));
        kfree(result);
    }
    
    uint64_t inference_time = get_system_time() - start_time;
    ai_system.total_inference_time += inference_time;
    
    model->state = AI_MODEL_STATE_TRAINED;
    
    spinlock_release(&ai_system.lock);
    
    return 0;
}

ai_dataset_t* ai_dataset_create(const char* name, ai_task_type_t task_type) {
    if (!ai_system.initialized || !name) {
        return NULL;
    }
    
    spinlock_acquire(&ai_system.lock);
    
    // Check if dataset already exists
    for (uint32_t i = 0; i < ai_system.dataset_count; i++) {
        if (ai_system.datasets[i] && strcmp(ai_system.datasets[i]->name, name) == 0) {
            spinlock_release(&ai_system.lock);
            return NULL;
        }
    }
    
    // Allocate dataset structure
    ai_dataset_t* dataset = kmalloc(sizeof(ai_dataset_t));
    if (!dataset) {
        spinlock_release(&ai_system.lock);
        return NULL;
    }
    
    // Initialize dataset
    memset(dataset, 0, sizeof(ai_dataset_t));
    dataset->id = ai_system.dataset_count + 1;
    strncpy(dataset->name, name, MAX_DATASET_NAME_LENGTH - 1);
    dataset->task_type = task_type;
    dataset->data = NULL;
    dataset->data_size = 0;
    dataset->samples = 1000;  // Default
    dataset->features = 10;   // Default
    dataset->classes = 0;     // Default
    dataset->train_split = 70;    // 70% training
    dataset->validation_split = 15; // 15% validation
    dataset->test_split = 15;      // 15% test
    strncpy(dataset->description, "Dataset created by RaeenOS", 255);
    strncpy(dataset->source, "Generated", 127);
    strncpy(dataset->license, "MIT", 63);
    dataset->created_time = get_system_time();
    dataset->modified_time = dataset->created_time;
    dataset->preprocessed = false;
    dataset->normalized = false;
    dataset->balanced = false;
    dataset->missing_data_ratio = 0.0f;
    dataset->outlier_ratio = 0.0f;
    
    // Add to dataset list
    ai_system.datasets[ai_system.dataset_count++] = dataset;
    
    spinlock_release(&ai_system.lock);
    
    printf("Created AI dataset: %s (task: %d, samples: %d)\n", name, task_type, dataset->samples);
    
    return dataset;
}

ai_task_t* ai_task_create(const char* name, ai_task_type_t type) {
    if (!ai_system.initialized || !name) {
        return NULL;
    }
    
    spinlock_acquire(&ai_system.lock);
    
    // Check if task already exists
    for (uint32_t i = 0; i < ai_system.task_count; i++) {
        if (ai_system.tasks[i] && strcmp(ai_system.tasks[i]->name, name) == 0) {
            spinlock_release(&ai_system.lock);
            return NULL;
        }
    }
    
    // Allocate task structure
    ai_task_t* task = kmalloc(sizeof(ai_task_t));
    if (!task) {
        spinlock_release(&ai_system.lock);
        return NULL;
    }
    
    // Initialize task
    memset(task, 0, sizeof(ai_task_t));
    task->id = ai_system.task_count + 1;
    strncpy(task->name, name, MAX_TASK_NAME_LENGTH - 1);
    task->type = type;
    task->running = false;
    task->completed = false;
    task->failed = false;
    task->model = NULL;
    task->dataset = NULL;
    task->parameters = NULL;
    task->parameter_count = 0;
    task->current_step = 0;
    task->total_steps = 100;  // Default
    task->progress = 0.0f;
    task->start_time = 0;
    task->estimated_completion = 0;
    task->results = NULL;
    task->results_size = 0;
    task->cpu_usage = 0.0f;
    task->memory_usage = 0.0f;
    task->gpu_usage = 0.0f;
    task->memory_allocated = 0;
    
    // Add to task list
    ai_system.tasks[ai_system.task_count++] = task;
    
    spinlock_release(&ai_system.lock);
    
    printf("Created AI task: %s (type: %d)\n", name, type);
    
    return task;
}

int ai_task_start(ai_task_t* task) {
    if (!task) {
        return -1;
    }
    
    spinlock_acquire(&ai_system.lock);
    
    if (task->running) {
        spinlock_release(&ai_system.lock);
        return -1;
    }
    
    task->running = true;
    task->completed = false;
    task->failed = false;
    task->current_step = 0;
    task->progress = 0.0f;
    task->start_time = get_system_time();
    task->estimated_completion = task->start_time + (task->total_steps * 100);  // 100ms per step
    
    // Add to running tasks
    if (ai_system.running_task_count < MAX_AI_TASKS) {
        ai_system.running_tasks[ai_system.running_task_count++] = task;
    }
    
    spinlock_release(&ai_system.lock);
    
    printf("Started AI task: %s\n", task->name);
    
    return 0;
}

ai_agent_t* ai_agent_create(const char* name, ai_agent_type_t type) {
    if (!ai_system.initialized || !name) {
        return NULL;
    }
    
    spinlock_acquire(&ai_system.lock);
    
    // Check if agent already exists
    for (uint32_t i = 0; i < ai_system.agent_count; i++) {
        if (ai_system.agents[i] && strcmp(ai_system.agents[i]->name, name) == 0) {
            spinlock_release(&ai_system.lock);
            return NULL;
        }
    }
    
    // Allocate agent structure
    ai_agent_t* agent = kmalloc(sizeof(ai_agent_t));
    if (!agent) {
        spinlock_release(&ai_system.lock);
        return NULL;
    }
    
    // Initialize agent
    memset(agent, 0, sizeof(ai_agent_t));
    agent->id = ai_system.agent_count + 1;
    strncpy(agent->name, name, 63);
    agent->type = type;
    agent->active = false;
    agent->capabilities = NULL;
    agent->capability_count = 0;
    agent->model_count = 0;
    strncpy(agent->personality, "Helpful and efficient", 255);
    strncpy(agent->goals, "Assist users and optimize system performance", 255);
    strncpy(agent->constraints, "Follow security policies and respect user privacy", 255);
    agent->can_communicate = true;
    strncpy(agent->communication_protocol, "natural_language", 31);
    agent->communication_data = NULL;
    agent->can_learn = true;
    agent->learning_data = NULL;
    agent->learning_rate = 0.001f;
    
    // Add to agent list
    ai_system.agents[ai_system.agent_count++] = agent;
    
    spinlock_release(&ai_system.lock);
    
    printf("Created AI agent: %s (type: %d)\n", name, type);
    
    return agent;
}

int ai_agent_communicate(ai_agent_t* agent, const char* message, char* response) {
    if (!agent || !message || !response) {
        return -1;
    }
    
    if (!agent->can_communicate) {
        return -1;
    }
    
    // Simple response simulation based on agent type
    switch (agent->type) {
        case AI_AGENT_TYPE_ASSISTANT:
            snprintf(response, 256, "Hello! I'm %s, your AI assistant. I received: '%s'. How can I help you?", 
                     agent->name, message);
            break;
        case AI_AGENT_TYPE_ANALYST:
            snprintf(response, 256, "I'm %s, the AI analyst. I'm analyzing: '%s'. The data shows interesting patterns.", 
                     agent->name, message);
            break;
        case AI_AGENT_TYPE_OPTIMIZER:
            snprintf(response, 256, "I'm %s, the AI optimizer. I'm optimizing based on: '%s'. Performance improvements detected.", 
                     agent->name, message);
            break;
        case AI_AGENT_TYPE_MONITOR:
            snprintf(response, 256, "I'm %s, the AI monitor. I'm monitoring: '%s'. All systems are operational.", 
                     agent->name, message);
            break;
        case AI_AGENT_TYPE_SCHEDULER:
            snprintf(response, 256, "I'm %s, the AI scheduler. I'm scheduling: '%s'. Task queued successfully.", 
                     agent->name, message);
            break;
        case AI_AGENT_TYPE_SECURITY:
            snprintf(response, 256, "I'm %s, the AI security agent. I'm analyzing: '%s'. No security threats detected.", 
                     agent->name, message);
            break;
        default:
            snprintf(response, 256, "I'm %s, an AI agent. I received: '%s'.", agent->name, message);
            break;
    }
    
    return 0;
}

ai_api_t* ai_api_create(const char* name, ai_api_type_t type) {
    if (!ai_system.initialized || !name) {
        return NULL;
    }
    
    spinlock_acquire(&ai_system.lock);
    
    // Check if API already exists
    for (uint32_t i = 0; i < ai_system.api_count; i++) {
        if (ai_system.apis[i] && strcmp(ai_system.apis[i]->name, name) == 0) {
            spinlock_release(&ai_system.lock);
            return NULL;
        }
    }
    
    // Allocate API structure
    ai_api_t* api = kmalloc(sizeof(ai_api_t));
    if (!api) {
        spinlock_release(&ai_system.lock);
        return NULL;
    }
    
    // Initialize API
    memset(api, 0, sizeof(ai_api_t));
    api->id = ai_system.api_count + 1;
    strncpy(api->name, name, MAX_API_NAME_LENGTH - 1);
    api->type = type;
    api->enabled = true;
    strncpy(api->base_url, "https://api.raeenos.ai", 255);
    api->endpoint_count = 0;
    api->requires_auth = true;
    strncpy(api->auth_type, "api_key", 31);
    strncpy(api->api_key, "demo_key_12345", 127);
    strncpy(api->auth_token, "", 255);
    api->rate_limit = 1000;  // 1000 requests per minute
    api->timeout = 30;       // 30 seconds
    api->retry_on_failure = true;
    api->max_retries = 3;
    api->requests_made = 0;
    api->successful_requests = 0;
    api->failed_requests = 0;
    api->average_response_time = 0.0f;
    
    // Add to API list
    ai_system.apis[ai_system.api_count++] = api;
    
    spinlock_release(&ai_system.lock);
    
    printf("Created AI API: %s (type: %d)\n", name, type);
    
    return api;
}

int ai_api_request(ai_api_t* api, const char* endpoint, void* request_data, void* response_data) {
    if (!api || !api->enabled || !endpoint) {
        return -1;
    }
    
    spinlock_acquire(&ai_system.lock);
    
    api->requests_made++;
    uint64_t start_time = get_system_time();
    
    // Simulate API request
    // In a real implementation, this would make actual HTTP requests
    
    // Simulate response time (50-200ms)
    uint64_t response_time = 50 + (get_system_time() % 150);
    
    // Simulate success/failure (90% success rate)
    bool success = (get_system_time() % 100) < 90;
    
    if (success) {
        api->successful_requests++;
        // Simulate response data
        if (response_data) {
            char* response = (char*)response_data;
            snprintf(response, 256, "{\"status\":\"success\",\"data\":\"AI response from %s\",\"timestamp\":%llu}", 
                     api->name, get_system_time());
        }
    } else {
        api->failed_requests++;
        if (response_data) {
            char* response = (char*)response_data;
            snprintf(response, 256, "{\"status\":\"error\",\"message\":\"Request failed\",\"timestamp\":%llu}", 
                     get_system_time());
        }
    }
    
    // Update average response time
    api->average_response_time = (api->average_response_time * (api->requests_made - 1) + response_time) / api->requests_made;
    
    spinlock_release(&ai_system.lock);
    
    return success ? 0 : -1;
}

void ai_system_dump_models(void) {
    if (!ai_system.initialized) {
        printf("AI system not initialized\n");
        return;
    }
    
    printf("\n=== AI Models ===\n");
    printf("Total models: %d\n", ai_system.model_count);
    printf("Active models: %d\n", ai_system.active_model_count);
    
    for (uint32_t i = 0; i < ai_system.model_count; i++) {
        if (ai_system.models[i]) {
            ai_model_t* model = ai_system.models[i];
            printf("\nModel %d: %s\n", i + 1, model->name);
            printf("  ID: %d, Type: %d, Task: %d, State: %d\n", 
                   model->id, model->type, model->task_type, model->state);
            printf("  Features: %d input, %d output\n", model->input_features, model->output_features);
            printf("  Samples: %d training, %d validation, %d test\n", 
                   model->training_samples, model->validation_samples, model->test_samples);
            printf("  Performance: accuracy=%.2f%%, precision=%.2f%%, recall=%.2f%%, f1=%.2f%%\n",
                   model->accuracy * 100.0f, model->precision * 100.0f, 
                   model->recall * 100.0f, model->f1_score * 100.0f);
            printf("  Loss: training=%.4f, validation=%.4f\n", model->loss, model->validation_loss);
            printf("  Training: %d epochs, %d batch size, %.4f learning rate\n",
                   model->epochs, model->batch_size, model->learning_rate);
            printf("  Hardware: GPU=%s, Quantized=%s, GPU Memory=%dMB\n",
                   model->gpu_accelerated ? "yes" : "no", 
                   model->quantized ? "yes" : "no", model->gpu_memory_usage);
            printf("  Version: %s, Author: %s\n", model->version, model->author);
        }
    }
    printf("================\n\n");
}

void ai_system_dump_tasks(void) {
    printf("\n=== AI Tasks ===\n");
    printf("Total tasks: %d\n", ai_system.task_count);
    printf("Running tasks: %d\n", ai_system.running_task_count);
    
    for (uint32_t i = 0; i < ai_system.task_count; i++) {
        if (ai_system.tasks[i]) {
            ai_task_t* task = ai_system.tasks[i];
            printf("\nTask %d: %s\n", i + 1, task->name);
            printf("  ID: %d, Type: %d\n", task->id, task->type);
            printf("  Status: running=%s, completed=%s, failed=%s\n",
                   task->running ? "yes" : "no", 
                   task->completed ? "yes" : "no", 
                   task->failed ? "yes" : "no");
            printf("  Progress: %d/%d steps (%.1f%%)\n", 
                   task->current_step, task->total_steps, task->progress * 100.0f);
            printf("  Performance: CPU=%.1f%%, Memory=%.1f%%, GPU=%.1f%%\n",
                   task->cpu_usage, task->memory_usage, task->gpu_usage);
            printf("  Memory allocated: %llu bytes\n", task->memory_allocated);
        }
    }
    printf("================\n\n");
}

void ai_system_dump_agents(void) {
    printf("\n=== AI Agents ===\n");
    printf("Total agents: %d\n", ai_system.agent_count);
    
    for (uint32_t i = 0; i < ai_system.agent_count; i++) {
        if (ai_system.agents[i]) {
            ai_agent_t* agent = ai_system.agents[i];
            printf("\nAgent %d: %s\n", i + 1, agent->name);
            printf("  ID: %d, Type: %d, Active: %s\n", 
                   agent->id, agent->type, agent->active ? "yes" : "no");
            printf("  Capabilities: %d, Models: %d\n", agent->capability_count, agent->model_count);
            printf("  Communication: %s (%s)\n", 
                   agent->can_communicate ? "enabled" : "disabled", agent->communication_protocol);
            printf("  Learning: %s (rate: %.4f)\n", 
                   agent->can_learn ? "enabled" : "disabled", agent->learning_rate);
            printf("  Personality: %s\n", agent->personality);
            printf("  Goals: %s\n", agent->goals);
            printf("  Constraints: %s\n", agent->constraints);
        }
    }
    printf("==================\n\n");
}

void ai_system_dump_apis(void) {
    printf("\n=== AI APIs ===\n");
    printf("Total APIs: %d\n", ai_system.api_count);
    
    for (uint32_t i = 0; i < ai_system.api_count; i++) {
        if (ai_system.apis[i]) {
            ai_api_t* api = ai_system.apis[i];
            printf("\nAPI %d: %s\n", i + 1, api->name);
            printf("  ID: %d, Type: %d, Enabled: %s\n", 
                   api->id, api->type, api->enabled ? "yes" : "no");
            printf("  Base URL: %s\n", api->base_url);
            printf("  Endpoints: %d\n", api->endpoint_count);
            printf("  Authentication: %s (%s)\n", 
                   api->requires_auth ? "required" : "none", api->auth_type);
            printf("  Rate limit: %d req/min, Timeout: %ds\n", api->rate_limit, api->timeout);
            printf("  Retry: %s (max %d)\n", 
                   api->retry_on_failure ? "enabled" : "disabled", api->max_retries);
            printf("  Statistics: %llu requests, %llu success, %llu failed\n",
                   api->requests_made, api->successful_requests, api->failed_requests);
            printf("  Average response time: %.2fms\n", api->average_response_time);
        }
    }
    printf("================\n\n");
} 