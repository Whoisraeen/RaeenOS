#ifndef INTELLIGENT_PERFORMANCE_H
#define INTELLIGENT_PERFORMANCE_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

// Intelligent performance constants
#define MAX_AI_MODELS 20
#define MAX_PREDICTIVE_RULES 100
#define MAX_ANALYTICS_METRICS 50
#define MAX_TROUBLESHOOTING_SCENARIOS 200
#define MAX_PERFORMANCE_PROFILES 30
#define MAX_OPTIMIZATION_STRATEGIES 40
#define MAX_LEARNING_PATTERNS 1000
#define MAX_PERFORMANCE_HISTORY 10000

// AI model types
typedef enum {
    AI_MODEL_PERFORMANCE = 0,
    AI_MODEL_PREDICTIVE = 1,
    AI_MODEL_OPTIMIZATION = 2,
    AI_MODEL_TROUBLESHOOTING = 3,
    AI_MODEL_USER_BEHAVIOR = 4,
    AI_MODEL_SYSTEM_HEALTH = 5
} ai_model_type_t;

// Predictive rule types
typedef enum {
    PREDICTIVE_RULE_CACHE = 0,
    PREDICTIVE_RULE_FAILURE = 1,
    PREDICTIVE_RULE_BACKUP = 2,
    PREDICTIVE_RULE_RESOURCE = 3,
    PREDICTIVE_RULE_SECURITY = 4,
    PREDICTIVE_RULE_MAINTENANCE = 5
} predictive_rule_type_t;

// Analytics metric types
typedef enum {
    ANALYTICS_CPU_USAGE = 0,
    ANALYTICS_MEMORY_USAGE = 1,
    ANALYTICS_DISK_USAGE = 2,
    ANALYTICS_NETWORK_USAGE = 3,
    ANALYTICS_GPU_USAGE = 4,
    ANALYTICS_APPLICATION_PERFORMANCE = 5,
    ANALYTICS_SYSTEM_RESPONSIVENESS = 6,
    ANALYTICS_BATTERY_LIFE = 7,
    ANALYTICS_TEMPERATURE = 8,
    ANALYTICS_ERROR_RATE = 9
} analytics_metric_type_t;

// Troubleshooting scenario types
typedef enum {
    TROUBLESHOOTING_PERFORMANCE = 0,
    TROUBLESHOOTING_STABILITY = 1,
    TROUBLESHOOTING_SECURITY = 2,
    TROUBLESHOOTING_COMPATIBILITY = 3,
    TROUBLESHOOTING_HARDWARE = 4,
    TROUBLESHOOTING_NETWORK = 5
} troubleshooting_scenario_type_t;

// Performance profile types
typedef enum {
    PERFORMANCE_PROFILE_POWER_SAVER = 0,
    PERFORMANCE_PROFILE_BALANCED = 1,
    PERFORMANCE_PROFILE_HIGH_PERFORMANCE = 2,
    PERFORMANCE_PROFILE_GAMING = 3,
    PERFORMANCE_PROFILE_CREATIVE = 4,
    PERFORMANCE_PROFILE_ENTERPRISE = 5,
    PERFORMANCE_PROFILE_CUSTOM = 6
} performance_profile_type_t;

// Optimization strategy types
typedef enum {
    OPTIMIZATION_CPU = 0,
    OPTIMIZATION_MEMORY = 1,
    OPTIMIZATION_DISK = 2,
    OPTIMIZATION_NETWORK = 3,
    OPTIMIZATION_GPU = 4,
    OPTIMIZATION_BATTERY = 5,
    OPTIMIZATION_TEMPERATURE = 6,
    OPTIMIZATION_SECURITY = 7
} optimization_strategy_type_t;

// AI model
typedef struct ai_model {
    uint32_t model_id;                           // Model identifier
    char name[64];                               // Model name
    char description[256];                       // Model description
    ai_model_type_t type;                        // Model type
    bool active;                                 // Model active
    bool trained;                                // Model trained
    float accuracy;                              // Model accuracy (0.0-1.0)
    uint64_t training_data_size;                 // Training data size
    uint64_t last_training;                      // Last training time
    uint64_t last_prediction;                    // Last prediction time
    uint32_t prediction_count;                   // Prediction count
    void* model_data;                            // Model-specific data
    void* weights;                               // Model weights
    uint32_t weights_size;                       // Weights size
} ai_model_t;

// Predictive rule
typedef struct predictive_rule {
    uint32_t rule_id;                            // Rule identifier
    char name[64];                               // Rule name
    char description[256];                       // Rule description
    predictive_rule_type_t type;                 // Rule type
    bool enabled;                                // Rule enabled
    bool active;                                 // Rule active
    float confidence;                            // Rule confidence (0.0-1.0)
    uint32_t trigger_threshold;                  // Trigger threshold
    uint32_t action_delay;                       // Action delay (ms)
    uint64_t last_triggered;                     // Last triggered time
    uint32_t trigger_count;                      // Trigger count
    void* condition_function;                    // Condition function pointer
    void* action_function;                       // Action function pointer
    void* rule_data;                             // Rule-specific data
} predictive_rule_t;

// Analytics metric
typedef struct analytics_metric {
    uint32_t metric_id;                          // Metric identifier
    char name[64];                               // Metric name
    char description[256];                       // Metric description
    analytics_metric_type_t type;                // Metric type
    bool enabled;                                // Metric enabled
    bool real_time;                              // Real-time monitoring
    float current_value;                         // Current value
    float min_value;                             // Minimum value
    float max_value;                             // Maximum value
    float average_value;                         // Average value
    uint64_t last_update;                        // Last update time
    uint64_t collection_interval;                // Collection interval (ms)
    uint32_t history_size;                       // History size
    float* history_data;                         // History data
    void* metric_data;                           // Metric-specific data
} analytics_metric_t;

// Troubleshooting scenario
typedef struct troubleshooting_scenario {
    uint32_t scenario_id;                        // Scenario identifier
    char name[64];                               // Scenario name
    char description[256];                       // Scenario description
    troubleshooting_scenario_type_t type;        // Scenario type
    bool enabled;                                // Scenario enabled
    bool active;                                 // Scenario active
    float severity;                              // Severity level (0.0-1.0)
    uint32_t detection_time;                     // Detection time (ms)
    uint32_t resolution_time;                    // Resolution time (ms)
    uint64_t last_occurred;                      // Last occurred time
    uint32_t occurrence_count;                   // Occurrence count
    bool auto_resolve;                           // Auto resolve enabled
    void* detection_function;                    // Detection function pointer
    void* resolution_function;                   // Resolution function pointer
    void* scenario_data;                         // Scenario-specific data
} troubleshooting_scenario_t;

// Performance profile
typedef struct performance_profile {
    uint32_t profile_id;                         // Profile identifier
    char name[64];                               // Profile name
    char description[256];                       // Profile description
    performance_profile_type_t type;             // Profile type
    bool active;                                 // Profile active
    bool custom;                                 // Custom profile
    uint32_t cpu_priority;                       // CPU priority (0-31)
    uint32_t memory_limit;                       // Memory limit (MB)
    uint32_t disk_priority;                      // Disk priority (0-31)
    uint32_t network_priority;                   // Network priority (0-31)
    uint32_t gpu_priority;                       // GPU priority (0-31)
    bool power_saving;                           // Power saving enabled
    bool thermal_management;                     // Thermal management enabled
    uint32_t target_fps;                         // Target FPS
    uint32_t target_latency;                     // Target latency (ms)
    void* profile_data;                          // Profile-specific data
} performance_profile_t;

// Optimization strategy
typedef struct optimization_strategy {
    uint32_t strategy_id;                        // Strategy identifier
    char name[64];                               // Strategy name
    char description[256];                       // Strategy description
    optimization_strategy_type_t type;           // Strategy type
    bool enabled;                                // Strategy enabled
    bool active;                                 // Strategy active
    float effectiveness;                         // Effectiveness (0.0-1.0)
    uint32_t application_time;                   // Application time (ms)
    uint32_t cooldown_time;                      // Cooldown time (ms)
    uint64_t last_applied;                       // Last applied time
    uint32_t application_count;                  // Application count
    bool adaptive;                               // Adaptive strategy
    void* apply_function;                        // Apply function pointer
    void* revert_function;                       // Revert function pointer
    void* strategy_data;                         // Strategy-specific data
} optimization_strategy_t;

// Learning pattern
typedef struct learning_pattern {
    uint32_t pattern_id;                         // Pattern identifier
    char name[64];                               // Pattern name
    char description[256];                       // Pattern description
    uint64_t timestamp;                          // Pattern timestamp
    uint32_t duration;                           // Pattern duration (ms)
    uint32_t frequency;                          // Pattern frequency
    float confidence;                            // Pattern confidence (0.0-1.0)
    uint32_t user_id;                            // Associated user
    uint32_t application_id;                     // Associated application
    void* pattern_data;                          // Pattern-specific data
    uint32_t data_size;                          // Pattern data size
} learning_pattern_t;

// Performance history entry
typedef struct performance_history_entry {
    uint64_t timestamp;                          // Entry timestamp
    float cpu_usage;                             // CPU usage (%)
    float memory_usage;                          // Memory usage (%)
    float disk_usage;                            // Disk usage (%)
    float network_usage;                         // Network usage (%)
    float gpu_usage;                             // GPU usage (%)
    uint32_t temperature;                        // Temperature (°C)
    uint32_t battery_level;                      // Battery level (%)
    uint32_t active_processes;                   // Active processes
    uint32_t system_responsiveness;              // System responsiveness (ms)
    uint32_t error_count;                        // Error count
    void* entry_data;                            // Entry-specific data
} performance_history_entry_t;

// Intelligent performance system
typedef struct intelligent_performance_system {
    spinlock_t lock;                             // System lock
    bool initialized;                            // Initialization flag
    
    // AI model management
    ai_model_t ai_models[MAX_AI_MODELS];         // AI models
    uint32_t ai_model_count;                     // Number of AI models
    
    // Predictive rule management
    predictive_rule_t predictive_rules[MAX_PREDICTIVE_RULES]; // Predictive rules
    uint32_t predictive_rule_count;              // Number of predictive rules
    
    // Analytics management
    analytics_metric_t analytics_metrics[MAX_ANALYTICS_METRICS]; // Analytics metrics
    uint32_t analytics_metric_count;             // Number of analytics metrics
    
    // Troubleshooting management
    troubleshooting_scenario_t troubleshooting_scenarios[MAX_TROUBLESHOOTING_SCENARIOS]; // Troubleshooting scenarios
    uint32_t troubleshooting_scenario_count;     // Number of troubleshooting scenarios
    
    // Performance profile management
    performance_profile_t performance_profiles[MAX_PERFORMANCE_PROFILES]; // Performance profiles
    uint32_t performance_profile_count;          // Number of performance profiles
    performance_profile_t* active_profile;       // Active performance profile
    
    // Optimization strategy management
    optimization_strategy_t optimization_strategies[MAX_OPTIMIZATION_STRATEGIES]; // Optimization strategies
    uint32_t optimization_strategy_count;        // Number of optimization strategies
    
    // Learning pattern management
    learning_pattern_t learning_patterns[MAX_LEARNING_PATTERNS]; // Learning patterns
    uint32_t learning_pattern_count;             // Number of learning patterns
    uint32_t learning_pattern_index;             // Learning pattern index
    
    // Performance history management
    performance_history_entry_t performance_history[MAX_PERFORMANCE_HISTORY]; // Performance history
    uint32_t performance_history_count;          // Number of performance history entries
    uint32_t performance_history_index;          // Performance history index
    
    // System configuration
    bool ai_optimization_enabled;                // AI optimization enabled
    bool predictive_systems_enabled;             // Predictive systems enabled
    bool analytics_enabled;                      // Analytics enabled
    bool smart_troubleshooting_enabled;          // Smart troubleshooting enabled
    bool learning_enabled;                       // Learning enabled
    bool adaptive_optimization_enabled;          // Adaptive optimization enabled
    
    // Performance monitoring
    bool real_time_monitoring;                   // Real-time monitoring
    uint32_t monitoring_interval;                // Monitoring interval (ms)
    bool performance_alerts;                     // Performance alerts enabled
    uint32_t alert_threshold;                    // Alert threshold
    
    // AI training
    bool auto_training_enabled;                  // Auto training enabled
    uint64_t training_interval;                  // Training interval (ms)
    uint64_t last_training;                      // Last training time
    bool incremental_learning;                   // Incremental learning enabled
    
    // Statistics
    uint64_t optimizations_applied;              // Optimizations applied
    uint64_t predictions_made;                   // Predictions made
    uint64_t issues_resolved;                    // Issues resolved
    uint64_t performance_improvements;           // Performance improvements
    uint64_t last_update;                        // Last update time
} intelligent_performance_system_t;

// Function declarations

// System initialization
int intelligent_performance_init(void);
void intelligent_performance_shutdown(void);
intelligent_performance_system_t* intelligent_performance_get_system(void);

// AI model management
ai_model_t* ai_model_create(const char* name, ai_model_type_t type);
int ai_model_destroy(uint32_t model_id);
int ai_model_train(uint32_t model_id, void* training_data, uint32_t data_size);
int ai_model_predict(uint32_t model_id, void* input_data, void* output_data);
int ai_model_evaluate(uint32_t model_id, float* accuracy);
ai_model_t* ai_model_find(uint32_t model_id);
ai_model_t* ai_model_find_by_name(const char* name);

// Predictive rule management
predictive_rule_t* predictive_rule_create(const char* name, predictive_rule_type_t type);
int predictive_rule_destroy(uint32_t rule_id);
int predictive_rule_enable(uint32_t rule_id, bool enabled);
int predictive_rule_set_confidence(uint32_t rule_id, float confidence);
int predictive_rule_trigger(uint32_t rule_id);
predictive_rule_t* predictive_rule_find(uint32_t rule_id);
predictive_rule_t* predictive_rule_find_by_name(const char* name);

// Analytics management
analytics_metric_t* analytics_metric_create(const char* name, analytics_metric_type_t type);
int analytics_metric_destroy(uint32_t metric_id);
int analytics_metric_enable(uint32_t metric_id, bool enabled);
int analytics_metric_update(uint32_t metric_id, float value);
int analytics_metric_get_history(uint32_t metric_id, float* history, uint32_t* count);
analytics_metric_t* analytics_metric_find(uint32_t metric_id);
analytics_metric_t* analytics_metric_find_by_name(const char* name);

// Troubleshooting management
troubleshooting_scenario_t* troubleshooting_scenario_create(const char* name, troubleshooting_scenario_type_t type);
int troubleshooting_scenario_destroy(uint32_t scenario_id);
int troubleshooting_scenario_enable(uint32_t scenario_id, bool enabled);
int troubleshooting_scenario_detect(uint32_t scenario_id);
int troubleshooting_scenario_resolve(uint32_t scenario_id);
troubleshooting_scenario_t* troubleshooting_scenario_find(uint32_t scenario_id);
troubleshooting_scenario_t* troubleshooting_scenario_find_by_name(const char* name);

// Performance profile management
performance_profile_t* performance_profile_create(const char* name, performance_profile_type_t type);
int performance_profile_destroy(uint32_t profile_id);
int performance_profile_apply(uint32_t profile_id);
int performance_profile_customize(uint32_t profile_id, void* custom_settings);
performance_profile_t* performance_profile_find(uint32_t profile_id);
performance_profile_t* performance_profile_find_by_name(const char* name);
performance_profile_t* performance_profile_get_active(void);

// Optimization strategy management
optimization_strategy_t* optimization_strategy_create(const char* name, optimization_strategy_type_t type);
int optimization_strategy_destroy(uint32_t strategy_id);
int optimization_strategy_enable(uint32_t strategy_id, bool enabled);
int optimization_strategy_apply(uint32_t strategy_id);
int optimization_strategy_revert(uint32_t strategy_id);
optimization_strategy_t* optimization_strategy_find(uint32_t strategy_id);
optimization_strategy_t* optimization_strategy_find_by_name(const char* name);

// Learning pattern management
learning_pattern_t* learning_pattern_add(const char* name, uint64_t timestamp, uint32_t duration);
int learning_pattern_remove(uint32_t pattern_id);
int learning_pattern_analyze(uint32_t pattern_id);
learning_pattern_t* learning_pattern_find(uint32_t pattern_id);
learning_pattern_t* learning_pattern_find_by_name(const char* name);

// Performance history management
int performance_history_add(performance_history_entry_t* entry);
int performance_history_get_range(uint64_t start_time, uint64_t end_time, performance_history_entry_t* entries, uint32_t* count);
int performance_history_analyze_trends(void);
int performance_history_cleanup_old_entries(uint64_t cutoff_time);

// System configuration
int ai_optimization_enable(bool enabled);
int predictive_systems_enable(bool enabled);
int analytics_enable(bool enabled);
int smart_troubleshooting_enable(bool enabled);
int learning_enable(bool enabled);
int adaptive_optimization_enable(bool enabled);

// Performance monitoring
int real_time_monitoring_enable(bool enabled);
int monitoring_interval_set(uint32_t interval);
int performance_alerts_enable(bool enabled);
int alert_threshold_set(uint32_t threshold);

// AI training
int auto_training_enable(bool enabled);
int training_interval_set(uint64_t interval);
int incremental_learning_enable(bool enabled);
int ai_system_train_all_models(void);

// High-level optimization functions
int intelligent_optimize_system(void);
int intelligent_predict_failures(void);
int intelligent_analyze_performance(void);
int intelligent_troubleshoot_issues(void);
int intelligent_learn_user_patterns(void);
int intelligent_apply_optimizations(void);

// Performance analysis
int performance_analyze_bottlenecks(void);
int performance_generate_recommendations(void);
int performance_benchmark_system(void);
int performance_compare_profiles(void);

// Statistics
void intelligent_performance_get_stats(intelligent_performance_system_t* stats);
void intelligent_performance_reset_stats(void);

#endif // INTELLIGENT_PERFORMANCE_H 