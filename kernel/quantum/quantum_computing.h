#ifndef QUANTUM_COMPUTING_H
#define QUANTUM_COMPUTING_H

#include "kernel.h"
#include <stdint.h>
#include <stdbool.h>

// Quantum computing constants
#define MAX_QUANTUM_DEVICES 10
#define MAX_QUANTUM_ALGORITHMS 50
#define MAX_QUANTUM_CIRCUITS 100
#define MAX_QUANTUM_QUBITS 1024
#define MAX_QUANTUM_GATES 1000
#define MAX_QUANTUM_JOBS 100
#define MAX_QUANTUM_RESULTS 1000

// Quantum device types
typedef enum {
    QUANTUM_DEVICE_SIMULATOR = 0,
    QUANTUM_DEVICE_SUPERCONDUCTING = 1,
    QUANTUM_DEVICE_ION_TRAP = 2,
    QUANTUM_DEVICE_PHOTONIC = 3,
    QUANTUM_DEVICE_TOPOLOGICAL = 4,
    QUANTUM_DEVICE_HYBRID = 5
} quantum_device_type_t;

// Quantum algorithm types
typedef enum {
    QUANTUM_ALGO_SHOR = 0,           // Shor's factoring algorithm
    QUANTUM_ALGO_GROVER = 1,         // Grover's search algorithm
    QUANTUM_ALGO_QFT = 2,            // Quantum Fourier Transform
    QUANTUM_ALGO_VQE = 3,            // Variational Quantum Eigensolver
    QUANTUM_ALGO_QAOA = 4,           // Quantum Approximate Optimization
    QUANTUM_ALGO_QML = 5,            // Quantum Machine Learning
    QUANTUM_ALGO_CRYPTO = 6,         // Quantum Cryptography
    QUANTUM_ALGO_SIMULATION = 7,     // Quantum Simulation
    QUANTUM_ALGO_OPTIMIZATION = 8,   // Quantum Optimization
    QUANTUM_ALGO_CUSTOM = 9          // Custom algorithm
} quantum_algorithm_type_t;

// Quantum gate types
typedef enum {
    QUANTUM_GATE_H = 0,              // Hadamard gate
    QUANTUM_GATE_X = 1,              // Pauli-X gate
    QUANTUM_GATE_Y = 2,              // Pauli-Y gate
    QUANTUM_GATE_Z = 3,              // Pauli-Z gate
    QUANTUM_GATE_CNOT = 4,           // Controlled-NOT gate
    QUANTUM_GATE_SWAP = 5,           // SWAP gate
    QUANTUM_GATE_PHASE = 6,          // Phase gate
    QUANTUM_GATE_T = 7,              // T gate
    QUANTUM_GATE_S = 8,              // S gate
    QUANTUM_GATE_RX = 9,             // Rotation X gate
    QUANTUM_GATE_RY = 10,            // Rotation Y gate
    QUANTUM_GATE_RZ = 11,            // Rotation Z gate
    QUANTUM_GATE_CUSTOM = 12         // Custom gate
} quantum_gate_type_t;

// Quantum job states
typedef enum {
    QUANTUM_JOB_PENDING = 0,
    QUANTUM_JOB_RUNNING = 1,
    QUANTUM_JOB_COMPLETED = 2,
    QUANTUM_JOB_FAILED = 3,
    QUANTUM_JOB_CANCELLED = 4
} quantum_job_state_t;

// Quantum device information
typedef struct quantum_device {
    uint32_t device_id;                           // Unique device identifier
    char name[64];                                // Device name
    char description[256];                        // Device description
    quantum_device_type_t type;                   // Device type
    bool available;                               // Device available
    
    // Device capabilities
    uint32_t max_qubits;                          // Maximum number of qubits
    uint32_t available_qubits;                    // Available qubits
    uint32_t max_circuit_depth;                   // Maximum circuit depth
    uint32_t coherence_time;                      // Coherence time (microseconds)
    float gate_fidelity;                          // Gate fidelity (0-1)
    float readout_fidelity;                       // Readout fidelity (0-1)
    
    // Device performance
    uint32_t shots_per_second;                    // Shots per second
    uint32_t queue_length;                        // Current queue length
    uint32_t total_jobs;                          // Total jobs processed
    uint32_t successful_jobs;                     // Successful jobs
    uint32_t failed_jobs;                         // Failed jobs
    
    // Device status
    uint64_t last_calibration;                    // Last calibration time
    uint64_t next_calibration;                    // Next calibration time
    bool needs_calibration;                       // Needs calibration
    bool maintenance_mode;                        // Maintenance mode
    
    // Device configuration
    char backend_type[32];                        // Backend type
    char provider[32];                            // Quantum provider
    char api_version[16];                         // API version
    uint32_t timeout_seconds;                     // Job timeout
    
    void* device_data;                            // Device-specific data
} quantum_device_t;

// Quantum algorithm information
typedef struct quantum_algorithm {
    uint32_t algorithm_id;                        // Unique algorithm identifier
    char name[64];                                // Algorithm name
    char description[256];                        // Algorithm description
    quantum_algorithm_type_t type;                // Algorithm type
    bool enabled;                                 // Algorithm enabled
    
    // Algorithm parameters
    uint32_t required_qubits;                     // Required qubits
    uint32_t circuit_depth;                       // Circuit depth
    uint32_t shots_required;                      // Shots required
    uint32_t max_iterations;                      // Maximum iterations
    float convergence_threshold;                  // Convergence threshold
    
    // Algorithm performance
    uint64_t total_runs;                          // Total runs
    uint64_t successful_runs;                     // Successful runs
    float average_runtime;                        // Average runtime (seconds)
    float success_rate;                           // Success rate (0-1)
    
    // Algorithm configuration
    char implementation[256];                     // Implementation path
    char parameters[512];                         // Algorithm parameters
    bool optimized;                               // Optimized version
    bool parallelizable;                          // Parallelizable
    
    void* algorithm_data;                         // Algorithm-specific data
} quantum_algorithm_t;

// Quantum circuit information
typedef struct quantum_circuit {
    uint32_t circuit_id;                          // Unique circuit identifier
    char name[64];                                // Circuit name
    char description[256];                        // Circuit description
    uint32_t algorithm_id;                        // Associated algorithm
    
    // Circuit structure
    uint32_t num_qubits;                          // Number of qubits
    uint32_t num_gates;                           // Number of gates
    uint32_t depth;                               // Circuit depth
    uint32_t width;                               // Circuit width
    
    // Circuit gates
    struct {
        quantum_gate_type_t type;                 // Gate type
        uint32_t target_qubit;                    // Target qubit
        uint32_t control_qubit;                   // Control qubit (if applicable)
        float parameter;                          // Gate parameter
        uint32_t layer;                           // Layer in circuit
    } gates[MAX_QUANTUM_GATES];                   // Circuit gates
    
    // Circuit optimization
    bool optimized;                               // Circuit optimized
    uint32_t optimization_level;                  // Optimization level
    float optimization_score;                     // Optimization score
    
    void* circuit_data;                           // Circuit-specific data
} quantum_circuit_t;

// Quantum job information
typedef struct quantum_job {
    uint32_t job_id;                              // Unique job identifier
    char name[64];                                // Job name
    char description[256];                        // Job description
    uint32_t device_id;                           // Target device
    uint32_t algorithm_id;                        // Algorithm to run
    uint32_t circuit_id;                          // Circuit to execute
    
    // Job configuration
    quantum_job_state_t state;                    // Job state
    uint32_t shots;                               // Number of shots
    uint32_t max_qubits;                          // Maximum qubits to use
    uint32_t timeout_seconds;                     // Job timeout
    bool optimize_circuit;                        // Optimize circuit
    
    // Job execution
    uint64_t submission_time;                     // Submission time
    uint64_t start_time;                          // Start time
    uint64_t completion_time;                     // Completion time
    uint32_t actual_shots;                        // Actual shots executed
    uint32_t successful_shots;                    // Successful shots
    
    // Job results
    char result_data[1024];                       // Result data
    uint32_t result_size;                         // Result size
    float execution_time;                         // Execution time (seconds)
    float success_rate;                           // Success rate (0-1)
    
    // Job metadata
    char user_id[32];                             // User ID
    char project_id[32];                          // Project ID
    uint32_t priority;                            // Job priority
    char tags[256];                               // Job tags
    
    void* job_data;                               // Job-specific data
} quantum_job_t;

// Quantum result information
typedef struct quantum_result {
    uint32_t result_id;                           // Unique result identifier
    uint32_t job_id;                              // Associated job
    char name[64];                                // Result name
    
    // Result data
    uint32_t num_qubits;                          // Number of qubits
    uint32_t num_shots;                           // Number of shots
    uint64_t* counts;                             // Measurement counts
    float* probabilities;                         // Probabilities
    char* state_vector;                           // State vector (if available)
    
    // Result analysis
    float expectation_value;                      // Expectation value
    float variance;                               // Variance
    float fidelity;                               // Fidelity
    char* classical_data;                         // Classical data
    
    // Result metadata
    uint64_t timestamp;                           // Result timestamp
    char format[16];                              // Result format
    uint32_t data_size;                           // Data size
    bool compressed;                              // Data compressed
    
    void* result_data;                            // Result-specific data
} quantum_result_t;

// Quantum-safe cryptography
typedef struct quantum_safe_crypto {
    uint32_t crypto_id;                           // Unique crypto identifier
    char name[64];                                // Crypto algorithm name
    char description[256];                        // Description
    
    // Algorithm parameters
    uint32_t key_size;                            // Key size (bits)
    uint32_t signature_size;                      // Signature size (bytes)
    uint32_t security_level;                      // Security level (bits)
    bool post_quantum;                            // Post-quantum secure
    
    // Performance metrics
    uint32_t key_generation_time;                 // Key generation time (ms)
    uint32_t signing_time;                        // Signing time (ms)
    uint32_t verification_time;                   // Verification time (ms)
    uint32_t encryption_time;                     // Encryption time (ms)
    uint32_t decryption_time;                     // Decryption time (ms)
    
    // Implementation details
    char implementation[256];                     // Implementation path
    char parameters[512];                         // Algorithm parameters
    bool hardware_accelerated;                    // Hardware accelerated
    bool optimized;                               // Optimized implementation
    
    void* crypto_data;                            // Crypto-specific data
} quantum_safe_crypto_t;

// Quantum computing system
typedef struct quantum_computing_system {
    spinlock_t lock;                              // System lock
    bool initialized;                             // Initialization flag
    
    // Device management
    quantum_device_t devices[MAX_QUANTUM_DEVICES]; // Quantum devices
    uint32_t device_count;                        // Number of devices
    
    // Algorithm management
    quantum_algorithm_t algorithms[MAX_QUANTUM_ALGORITHMS]; // Quantum algorithms
    uint32_t algorithm_count;                     // Number of algorithms
    
    // Circuit management
    quantum_circuit_t circuits[MAX_QUANTUM_CIRCUITS]; // Quantum circuits
    uint32_t circuit_count;                       // Number of circuits
    
    // Job management
    quantum_job_t jobs[MAX_QUANTUM_JOBS];         // Quantum jobs
    uint32_t job_count;                           // Number of jobs
    uint32_t job_index;                           // Current job index
    
    // Result management
    quantum_result_t results[MAX_QUANTUM_RESULTS]; // Quantum results
    uint32_t result_count;                        // Number of results
    
    // Quantum-safe cryptography
    quantum_safe_crypto_t crypto_algorithms[20];  // Crypto algorithms
    uint32_t crypto_count;                        // Number of crypto algorithms
    
    // System configuration
    bool quantum_enabled;                         // Quantum computing enabled
    bool hybrid_enabled;                          // Hybrid classical-quantum enabled
    bool crypto_enabled;                          // Quantum-safe crypto enabled
    bool simulation_enabled;                      // Quantum simulation enabled
    
    // System statistics
    uint64_t total_jobs_processed;                // Total jobs processed
    uint64_t total_qubits_used;                   // Total qubits used
    uint64_t total_execution_time;                // Total execution time
    uint64_t last_update;                         // Last update time
} quantum_computing_system_t;

// Quantum computing statistics
typedef struct quantum_stats {
    // Device statistics
    uint32_t total_devices;                       // Total devices
    uint32_t available_devices;                   // Available devices
    uint32_t total_qubits;                        // Total qubits
    uint32_t available_qubits;                    // Available qubits
    
    // Algorithm statistics
    uint32_t total_algorithms;                    // Total algorithms
    uint32_t enabled_algorithms;                  // Enabled algorithms
    uint64_t total_algorithm_runs;                // Total algorithm runs
    float average_success_rate;                   // Average success rate
    
    // Job statistics
    uint32_t total_jobs;                          // Total jobs
    uint32_t pending_jobs;                        // Pending jobs
    uint32_t running_jobs;                        // Running jobs
    uint32_t completed_jobs;                      // Completed jobs
    uint32_t failed_jobs;                         // Failed jobs
    
    // Performance statistics
    float average_job_time;                       // Average job time
    float average_qubit_utilization;              // Average qubit utilization
    float system_throughput;                      // System throughput (jobs/sec)
    
    // Crypto statistics
    uint32_t total_crypto_algorithms;             // Total crypto algorithms
    uint32_t post_quantum_algorithms;             // Post-quantum algorithms
    uint64_t total_crypto_operations;             // Total crypto operations
    
    // System statistics
    uint64_t last_update;                         // Last update time
} quantum_stats_t;

// Function declarations

// System initialization
int quantum_computing_init(void);
void quantum_computing_shutdown(void);
quantum_computing_system_t* quantum_computing_get_system(void);

// Device management
quantum_device_t* quantum_device_add(const char* name, quantum_device_type_t type);
int quantum_device_remove(uint32_t device_id);
int quantum_device_calibrate(uint32_t device_id);
quantum_device_t* quantum_device_find(uint32_t device_id);
quantum_device_t* quantum_device_find_by_name(const char* name);

// Algorithm management
quantum_algorithm_t* quantum_algorithm_add(const char* name, quantum_algorithm_type_t type);
int quantum_algorithm_remove(uint32_t algorithm_id);
int quantum_algorithm_optimize(uint32_t algorithm_id);
quantum_algorithm_t* quantum_algorithm_find(uint32_t algorithm_id);
quantum_algorithm_t* quantum_algorithm_find_by_name(const char* name);

// Circuit management
quantum_circuit_t* quantum_circuit_create(const char* name, uint32_t algorithm_id);
int quantum_circuit_destroy(uint32_t circuit_id);
int quantum_circuit_add_gate(uint32_t circuit_id, quantum_gate_type_t gate_type, uint32_t target_qubit);
int quantum_circuit_optimize(uint32_t circuit_id);
quantum_circuit_t* quantum_circuit_find(uint32_t circuit_id);
quantum_circuit_t* quantum_circuit_find_by_name(const char* name);

// Job management
quantum_job_t* quantum_job_submit(const char* name, uint32_t device_id, uint32_t algorithm_id);
int quantum_job_cancel(uint32_t job_id);
int quantum_job_get_status(uint32_t job_id);
quantum_job_t* quantum_job_find(uint32_t job_id);
quantum_job_t* quantum_job_find_by_name(const char* name);

// Result management
quantum_result_t* quantum_result_get(uint32_t job_id);
int quantum_result_analyze(uint32_t result_id);
int quantum_result_export(uint32_t result_id, const char* format);
quantum_result_t* quantum_result_find(uint32_t result_id);

// Quantum-safe cryptography
quantum_safe_crypto_t* quantum_crypto_add(const char* name, bool post_quantum);
int quantum_crypto_remove(uint32_t crypto_id);
int quantum_crypto_generate_key(uint32_t crypto_id, uint8_t* public_key, uint8_t* private_key);
int quantum_crypto_sign(uint32_t crypto_id, const uint8_t* data, uint32_t data_size, uint8_t* signature);
int quantum_crypto_verify(uint32_t crypto_id, const uint8_t* data, uint32_t data_size, const uint8_t* signature);
quantum_safe_crypto_t* quantum_crypto_find(uint32_t crypto_id);
quantum_safe_crypto_t* quantum_crypto_find_by_name(const char* name);

// System configuration
int quantum_computing_enable_quantum(bool enabled);
int quantum_computing_enable_hybrid(bool enabled);
int quantum_computing_enable_crypto(bool enabled);
int quantum_computing_enable_simulation(bool enabled);

// Monitoring and statistics
void quantum_computing_get_stats(quantum_stats_t* stats);
void quantum_computing_reset_stats(void);
int quantum_computing_perform_benchmark(void);
int quantum_computing_generate_report(void);

// Quantum computing debugging
void quantum_computing_dump_devices(void);
void quantum_computing_dump_algorithms(void);
void quantum_computing_dump_circuits(void);
void quantum_computing_dump_jobs(void);
void quantum_computing_dump_results(void);
void quantum_computing_dump_crypto(void);
void quantum_computing_dump_stats(void);

#endif // QUANTUM_COMPUTING_H 