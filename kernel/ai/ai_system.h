#ifndef AI_SYSTEM_H
#define AI_SYSTEM_H

#include "kernel.h"
#include <stdint.h>
#include <stdbool.h>

// AI system constants
#define MAX_AI_MODELS 50
#define MAX_AI_TASKS 100
#define MAX_AI_DATASETS 200
#define MAX_AI_PIPELINES 50
#define MAX_AI_AGENTS 20
#define MAX_AI_APIS 30
#define MAX_MODEL_NAME_LENGTH 64
#define MAX_DATASET_NAME_LENGTH 64
#define MAX_TASK_NAME_LENGTH 64
#define MAX_API_NAME_LENGTH 64

// AI model types
typedef enum {
    AI_MODEL_TYPE_NEURAL_NETWORK = 0,
    AI_MODEL_TYPE_DECISION_TREE = 1,
    AI_MODEL_TYPE_SUPPORT_VECTOR_MACHINE = 2,
    AI_MODEL_TYPE_RANDOM_FOREST = 3,
    AI_MODEL_TYPE_K_MEANS = 4,
    AI_MODEL_TYPE_NAIVE_BAYES = 5,
    AI_MODEL_TYPE_LINEAR_REGRESSION = 6,
    AI_MODEL_TYPE_LOGISTIC_REGRESSION = 7,
    AI_MODEL_TYPE_TRANSFORMER = 8,
    AI_MODEL_TYPE_CNN = 9,
    AI_MODEL_TYPE_RNN = 10,
    AI_MODEL_TYPE_LSTM = 11,
    AI_MODEL_TYPE_GRU = 12
} ai_model_type_t;

// AI task types
typedef enum {
    AI_TASK_CLASSIFICATION = 0,
    AI_TASK_REGRESSION = 1,
    AI_TASK_CLUSTERING = 2,
    AI_TASK_DIMENSIONALITY_REDUCTION = 3,
    AI_TASK_ANOMALY_DETECTION = 4,
    AI_TASK_RECOMMENDATION = 5,
    AI_TASK_NATURAL_LANGUAGE_PROCESSING = 6,
    AI_TASK_COMPUTER_VISION = 7,
    AI_TASK_SPEECH_RECOGNITION = 8,
    AI_TASK_SPEECH_SYNTHESIS = 9,
    AI_TASK_TRANSLATION = 10,
    AI_TASK_SUMMARIZATION = 11,
    AI_TASK_QUESTION_ANSWERING = 12,
    AI_TASK_SENTIMENT_ANALYSIS = 13,
    AI_TASK_OBJECT_DETECTION = 14,
    AI_TASK_IMAGE_SEGMENTATION = 15,
    AI_TASK_FACE_RECOGNITION = 16,
    AI_TASK_OPTICAL_CHARACTER_RECOGNITION = 17,
    AI_TASK_TIME_SERIES_FORECASTING = 18,
    AI_TASK_REINFORCEMENT_LEARNING = 19
} ai_task_type_t;

// AI model states
typedef enum {
    AI_MODEL_STATE_UNLOADED = 0,
    AI_MODEL_STATE_LOADING = 1,
    AI_MODEL_STATE_LOADED = 2,
    AI_MODEL_STATE_TRAINING = 3,
    AI_MODEL_STATE_TRAINED = 4,
    AI_MODEL_STATE_INFERENCE = 5,
    AI_MODEL_STATE_ERROR = 6
} ai_model_state_t;

// AI model information
typedef struct ai_model {
    uint32_t id;                            // Model ID
    char name[MAX_MODEL_NAME_LENGTH];       // Model name
    ai_model_type_t type;                   // Model type
    ai_model_state_t state;                 // Model state
    ai_task_type_t task_type;               // Task type
    
    // Model data
    void* model_data;                       // Model data
    uint64_t model_size;                    // Model size (bytes)
    char model_path[256];                   // Model file path
    char config_path[256];                  // Configuration file path
    
    // Training data
    uint32_t input_features;                // Number of input features
    uint32_t output_features;               // Number of output features
    uint32_t training_samples;              // Number of training samples
    uint32_t validation_samples;            // Number of validation samples
    uint32_t test_samples;                  // Number of test samples
    
    // Performance metrics
    float accuracy;                         // Model accuracy
    float precision;                        // Model precision
    float recall;                           // Model recall
    float f1_score;                         // F1 score
    float loss;                             // Training loss
    float validation_loss;                  // Validation loss
    
    // Training information
    uint32_t epochs;                        // Number of epochs
    uint32_t batch_size;                    // Batch size
    float learning_rate;                    // Learning rate
    uint64_t training_time;                 // Training time (ms)
    uint64_t last_training;                 // Last training time
    
    // Hardware acceleration
    bool gpu_accelerated;                   // GPU acceleration
    bool quantized;                         // Model quantization
    uint32_t gpu_memory_usage;              // GPU memory usage (MB)
    
    // Version and metadata
    char version[32];                       // Model version
    char author[64];                        // Model author
    char description[256];                  // Model description
    uint64_t created_time;                  // Creation time
    uint64_t modified_time;                 // Last modification time
    
    void* user_data;                        // User-specific data
} ai_model_t;

// AI dataset information
typedef struct ai_dataset {
    uint32_t id;                            // Dataset ID
    char name[MAX_DATASET_NAME_LENGTH];     // Dataset name
    ai_task_type_t task_type;               // Task type
    
    // Dataset data
    void* data;                             // Dataset data
    uint64_t data_size;                     // Data size (bytes)
    char data_path[256];                    // Data file path
    char format[32];                        // Data format (CSV, JSON, etc.)
    
    // Dataset structure
    uint32_t samples;                       // Number of samples
    uint32_t features;                      // Number of features
    uint32_t classes;                       // Number of classes (for classification)
    uint32_t train_split;                   // Training split percentage
    uint32_t validation_split;              // Validation split percentage
    uint32_t test_split;                    // Test split percentage
    
    // Dataset metadata
    char description[256];                  // Dataset description
    char source[128];                       // Data source
    char license[64];                       // License information
    uint64_t created_time;                  // Creation time
    uint64_t modified_time;                 // Last modification time
    
    // Data quality
    bool preprocessed;                      // Is preprocessed
    bool normalized;                        // Is normalized
    bool balanced;                          // Is balanced
    float missing_data_ratio;               // Missing data ratio
    float outlier_ratio;                    // Outlier ratio
    
    void* user_data;                        // User-specific data
} ai_dataset_t;

// AI task information
typedef struct ai_task {
    uint32_t id;                            // Task ID
    char name[MAX_TASK_NAME_LENGTH];        // Task name
    ai_task_type_t type;                    // Task type
    bool running;                           // Is running
    bool completed;                         // Is completed
    bool failed;                            // Has failed
    
    // Task configuration
    ai_model_t* model;                      // Associated model
    ai_dataset_t* dataset;                  // Associated dataset
    void* parameters;                       // Task parameters
    uint32_t parameter_count;               // Number of parameters
    
    // Task progress
    uint32_t current_step;                  // Current step
    uint32_t total_steps;                   // Total steps
    float progress;                         // Progress (0.0 - 1.0)
    uint64_t start_time;                    // Start time
    uint64_t estimated_completion;          // Estimated completion time
    
    // Task results
    void* results;                          // Task results
    uint64_t results_size;                  // Results size
    char results_path[256];                 // Results file path
    
    // Task performance
    float cpu_usage;                        // CPU usage (%)
    float memory_usage;                     // Memory usage (%)
    float gpu_usage;                        // GPU usage (%)
    uint64_t memory_allocated;              // Memory allocated (bytes)
    
    void* user_data;                        // User-specific data
} ai_task_t;

// AI pipeline information
typedef struct ai_pipeline {
    uint32_t id;                            // Pipeline ID
    char name[64];                          // Pipeline name
    bool active;                            // Is active
    
    // Pipeline stages
    ai_task_t* stages[MAX_AI_TASKS];        // Pipeline stages
    uint32_t stage_count;                   // Number of stages
    uint32_t current_stage;                 // Current stage
    
    // Pipeline configuration
    bool parallel_execution;                // Parallel execution
    bool error_recovery;                    // Error recovery
    uint32_t max_retries;                   // Maximum retries
    
    // Pipeline results
    void* final_results;                    // Final results
    uint64_t results_size;                  // Results size
    
    void* user_data;                        // User-specific data
} ai_pipeline_t;

// AI agent types
typedef enum {
    AI_AGENT_TYPE_ASSISTANT = 0,
    AI_AGENT_TYPE_ANALYST = 1,
    AI_AGENT_TYPE_OPTIMIZER = 2,
    AI_AGENT_TYPE_MONITOR = 3,
    AI_AGENT_TYPE_SCHEDULER = 4,
    AI_AGENT_TYPE_SECURITY = 5
} ai_agent_type_t;

// AI agent information
typedef struct ai_agent {
    uint32_t id;                            // Agent ID
    char name[64];                          // Agent name
    ai_agent_type_t type;                   // Agent type
    bool active;                            // Is active
    
    // Agent capabilities
    ai_task_type_t* capabilities;           // Task capabilities
    uint32_t capability_count;              // Number of capabilities
    ai_model_t* models[MAX_AI_MODELS];      // Associated models
    uint32_t model_count;                   // Number of models
    
    // Agent behavior
    char personality[256];                  // Agent personality
    char goals[256];                        // Agent goals
    char constraints[256];                  // Agent constraints
    
    // Agent communication
    bool can_communicate;                   // Can communicate
    char communication_protocol[32];        // Communication protocol
    void* communication_data;               // Communication data
    
    // Agent learning
    bool can_learn;                         // Can learn
    ai_dataset_t* learning_data;            // Learning data
    float learning_rate;                    // Learning rate
    
    void* user_data;                        // User-specific data
} ai_agent_t;

// AI API types
typedef enum {
    AI_API_TYPE_REST = 0,
    AI_API_TYPE_GRAPHQL = 1,
    AI_API_TYPE_GRPC = 2,
    AI_API_TYPE_WEBSOCKET = 3,
    AI_API_TYPE_MESSAGE_QUEUE = 4
} ai_api_type_t;

// AI API information
typedef struct ai_api {
    uint32_t id;                            // API ID
    char name[MAX_API_NAME_LENGTH];         // API name
    ai_api_type_t type;                     // API type
    bool enabled;                           // Is enabled
    
    // API endpoints
    char base_url[256];                     // Base URL
    char endpoints[10][64];                 // API endpoints
    uint32_t endpoint_count;                // Number of endpoints
    
    // API authentication
    bool requires_auth;                     // Requires authentication
    char auth_type[32];                     // Authentication type
    char api_key[128];                      // API key
    char auth_token[256];                   // Authentication token
    
    // API configuration
    uint32_t rate_limit;                    // Rate limit (requests per minute)
    uint32_t timeout;                       // Timeout (seconds)
    bool retry_on_failure;                  // Retry on failure
    uint32_t max_retries;                   // Maximum retries
    
    // API statistics
    uint64_t requests_made;                 // Requests made
    uint64_t successful_requests;           // Successful requests
    uint64_t failed_requests;               // Failed requests
    float average_response_time;            // Average response time (ms)
    
    void* user_data;                        // User-specific data
} ai_api_t;

// AI system
typedef struct ai_system {
    spinlock_t lock;                        // System lock
    bool initialized;                       // Initialization flag
    
    // Model management
    ai_model_t* models[MAX_AI_MODELS];      // AI models
    uint32_t model_count;                   // Number of models
    ai_model_t* active_models[MAX_AI_MODELS]; // Active models
    uint32_t active_model_count;            // Number of active models
    
    // Dataset management
    ai_dataset_t* datasets[MAX_AI_DATASETS]; // AI datasets
    uint32_t dataset_count;                 // Number of datasets
    
    // Task management
    ai_task_t* tasks[MAX_AI_TASKS];         // AI tasks
    uint32_t task_count;                    // Number of tasks
    ai_task_t* running_tasks[MAX_AI_TASKS]; // Running tasks
    uint32_t running_task_count;            // Number of running tasks
    
    // Pipeline management
    ai_pipeline_t* pipelines[MAX_AI_PIPELINES]; // AI pipelines
    uint32_t pipeline_count;                // Number of pipelines
    
    // Agent management
    ai_agent_t* agents[MAX_AI_AGENTS];      // AI agents
    uint32_t agent_count;                   // Number of agents
    
    // API management
    ai_api_t* apis[MAX_AI_APIS];            // AI APIs
    uint32_t api_count;                     // Number of APIs
    
    // System configuration
    bool gpu_acceleration;                  // GPU acceleration enabled
    bool distributed_computing;             // Distributed computing enabled
    bool cloud_integration;                 // Cloud integration enabled
    uint32_t max_concurrent_tasks;          // Maximum concurrent tasks
    uint32_t max_memory_usage;              // Maximum memory usage (MB)
    
    // System performance
    float cpu_usage;                        // CPU usage (%)
    float memory_usage;                     // Memory usage (%)
    float gpu_usage;                        // GPU usage (%)
    uint64_t total_inference_time;          // Total inference time (ms)
    uint64_t total_training_time;           // Total training time (ms)
} ai_system_t;

// AI system statistics
typedef struct ai_system_stats {
    // Model statistics
    uint32_t total_models;                  // Total models
    uint32_t loaded_models;                 // Loaded models
    uint32_t trained_models;                // Trained models
    uint32_t active_models;                 // Active models
    
    // Task statistics
    uint32_t total_tasks;                   // Total tasks
    uint32_t completed_tasks;               // Completed tasks
    uint32_t failed_tasks;                  // Failed tasks
    uint32_t running_tasks;                 // Running tasks
    
    // Performance statistics
    uint64_t total_inference_requests;      // Total inference requests
    uint64_t successful_inference_requests; // Successful inference requests
    uint64_t total_training_requests;       // Total training requests
    uint64_t successful_training_requests;  // Successful training requests
    float average_inference_time;           // Average inference time (ms)
    float average_training_time;            // Average training time (ms)
    
    // Resource statistics
    uint64_t total_memory_usage;            // Total memory usage (bytes)
    uint64_t total_gpu_memory_usage;        // Total GPU memory usage (bytes)
    float average_cpu_usage;                // Average CPU usage (%)
    float average_gpu_usage;                // Average GPU usage (%)
    
    // System statistics
    uint64_t last_update;                   // Last update time
} ai_system_stats_t;

// Function declarations

// System initialization
int ai_system_init(void);
void ai_system_shutdown(void);
ai_system_t* ai_system_get_system(void);

// Model management
ai_model_t* ai_model_create(const char* name, ai_model_type_t type, ai_task_type_t task_type);
int ai_model_destroy(ai_model_t* model);
int ai_model_load(ai_model_t* model, const char* model_path);
int ai_model_save(ai_model_t* model, const char* model_path);
int ai_model_train(ai_model_t* model, ai_dataset_t* dataset, void* parameters);
int ai_model_inference(ai_model_t* model, void* input, void* output);
int ai_model_evaluate(ai_model_t* model, ai_dataset_t* dataset);
ai_model_t* ai_model_find(const char* name);
ai_model_t* ai_model_find_by_id(uint32_t id);

// Dataset management
ai_dataset_t* ai_dataset_create(const char* name, ai_task_type_t task_type);
int ai_dataset_destroy(ai_dataset_t* dataset);
int ai_dataset_load(ai_dataset_t* dataset, const char* data_path);
int ai_dataset_save(ai_dataset_t* dataset, const char* data_path);
int ai_dataset_preprocess(ai_dataset_t* dataset);
int ai_dataset_split(ai_dataset_t* dataset, float train_ratio, float validation_ratio, float test_ratio);
ai_dataset_t* ai_dataset_find(const char* name);
ai_dataset_t* ai_dataset_find_by_id(uint32_t id);

// Task management
ai_task_t* ai_task_create(const char* name, ai_task_type_t type);
int ai_task_destroy(ai_task_t* task);
int ai_task_start(ai_task_t* task);
int ai_task_stop(ai_task_t* task);
int ai_task_pause(ai_task_t* task);
int ai_task_resume(ai_task_t* task);
int ai_task_get_progress(ai_task_t* task, float* progress);
ai_task_t* ai_task_find(const char* name);
ai_task_t* ai_task_find_by_id(uint32_t id);

// Pipeline management
ai_pipeline_t* ai_pipeline_create(const char* name);
int ai_pipeline_destroy(ai_pipeline_t* pipeline);
int ai_pipeline_add_stage(ai_pipeline_t* pipeline, ai_task_t* task);
int ai_pipeline_remove_stage(ai_pipeline_t* pipeline, uint32_t stage_index);
int ai_pipeline_start(ai_pipeline_t* pipeline);
int ai_pipeline_stop(ai_pipeline_t* pipeline);
ai_pipeline_t* ai_pipeline_find(const char* name);
ai_pipeline_t* ai_pipeline_find_by_id(uint32_t id);

// Agent management
ai_agent_t* ai_agent_create(const char* name, ai_agent_type_t type);
int ai_agent_destroy(ai_agent_t* agent);
int ai_agent_start(ai_agent_t* agent);
int ai_agent_stop(ai_agent_t* agent);
int ai_agent_communicate(ai_agent_t* agent, const char* message, char* response);
int ai_agent_learn(ai_agent_t* agent, ai_dataset_t* dataset);
ai_agent_t* ai_agent_find(const char* name);
ai_agent_t* ai_agent_find_by_id(uint32_t id);

// API management
ai_api_t* ai_api_create(const char* name, ai_api_type_t type);
int ai_api_destroy(ai_api_t* api);
int ai_api_configure(ai_api_t* api, const char* base_url, const char* auth_type, const char* api_key);
int ai_api_request(ai_api_t* api, const char* endpoint, void* request_data, void* response_data);
int ai_api_test_connection(ai_api_t* api);
ai_api_t* ai_api_find(const char* name);
ai_api_t* ai_api_find_by_id(uint32_t id);

// System configuration
int ai_system_set_gpu_acceleration(bool enabled);
int ai_system_set_distributed_computing(bool enabled);
int ai_system_set_cloud_integration(bool enabled);
int ai_system_set_max_concurrent_tasks(uint32_t max_tasks);
int ai_system_set_max_memory_usage(uint32_t max_memory);

// AI utilities
int ai_model_quantize(ai_model_t* model, uint8_t bits);
int ai_model_optimize(ai_model_t* model);
int ai_dataset_augment(ai_dataset_t* dataset);
int ai_dataset_balance(ai_dataset_t* dataset);
int ai_task_schedule(ai_task_t* task, uint64_t start_time);
int ai_pipeline_optimize(ai_pipeline_t* pipeline);

// AI system statistics
void ai_system_get_stats(ai_system_stats_t* stats);
void ai_system_reset_stats(void);

// AI system debugging
void ai_system_dump_models(void);
void ai_system_dump_datasets(void);
void ai_system_dump_tasks(void);
void ai_system_dump_pipelines(void);
void ai_system_dump_agents(void);
void ai_system_dump_apis(void);
void ai_system_dump_stats(void);

#endif // AI_SYSTEM_H 