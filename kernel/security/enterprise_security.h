#ifndef ENTERPRISE_SECURITY_H
#define ENTERPRISE_SECURITY_H

#include "security.h"
#include <stdint.h>
#include <stdbool.h>

// Enterprise security constants
#define MAX_SECURITY_POLICIES 100
#define MAX_COMPLIANCE_FRAMEWORKS 20
#define MAX_AUDIT_RECORDS 10000
#define MAX_ENCRYPTION_KEYS 1000
#define MAX_CERTIFICATES 500
#define MAX_SECURITY_ZONES 50
#define MAX_THREAT_INDICATORS 1000
#define MAX_INCIDENT_RECORDS 1000

// Compliance frameworks
typedef enum {
    COMPLIANCE_NONE = 0,
    COMPLIANCE_SOX = 1,           // Sarbanes-Oxley
    COMPLIANCE_HIPAA = 2,         // Health Insurance Portability and Accountability Act
    COMPLIANCE_PCI_DSS = 3,       // Payment Card Industry Data Security Standard
    COMPLIANCE_GDPR = 4,          // General Data Protection Regulation
    COMPLIANCE_ISO27001 = 5,      // ISO/IEC 27001 Information Security
    COMPLIANCE_NIST = 6,          // NIST Cybersecurity Framework
    COMPLIANCE_FEDRAMP = 7,       // Federal Risk and Authorization Management Program
    COMPLIANCE_CCPA = 8,          // California Consumer Privacy Act
    COMPLIANCE_SOC2 = 9,          // Service Organization Control 2
    COMPLIANCE_GLBA = 10,         // Gramm-Leach-Bliley Act
    COMPLIANCE_FISMA = 11,        // Federal Information Security Management Act
    COMPLIANCE_COBIT = 12,        // Control Objectives for Information and Related Technologies
    COMPLIANCE_ITIL = 13,         // Information Technology Infrastructure Library
    COMPLIANCE_CIS = 14,          // Center for Internet Security
    COMPLIANCE_OWASP = 15,        // Open Web Application Security Project
    COMPLIANCE_CSA = 16,          // Cloud Security Alliance
    COMPLIANCE_27017 = 17,        // ISO/IEC 27017 Cloud Security
    COMPLIANCE_27018 = 18,        // ISO/IEC 27018 Cloud Privacy
    COMPLIANCE_CUSTOM = 19        // Custom compliance framework
} compliance_framework_t;

// Security zones
typedef enum {
    SECURITY_ZONE_UNTRUSTED = 0,  // Internet, external networks
    SECURITY_ZONE_DMZ = 1,        // Demilitarized zone
    SECURITY_ZONE_TRUSTED = 2,    // Internal networks
    SECURITY_ZONE_RESTRICTED = 3, // Highly sensitive data
    SECURITY_ZONE_CRITICAL = 4,   // Critical infrastructure
    SECURITY_ZONE_ISOLATED = 5    // Air-gapped systems
} security_zone_t;

// Threat levels
typedef enum {
    THREAT_LEVEL_NONE = 0,
    THREAT_LEVEL_LOW = 1,
    THREAT_LEVEL_MEDIUM = 2,
    THREAT_LEVEL_HIGH = 3,
    THREAT_LEVEL_CRITICAL = 4
} threat_level_t;

// Incident severity
typedef enum {
    INCIDENT_SEVERITY_INFO = 0,
    INCIDENT_SEVERITY_LOW = 1,
    INCIDENT_SEVERITY_MEDIUM = 2,
    INCIDENT_SEVERITY_HIGH = 3,
    INCIDENT_SEVERITY_CRITICAL = 4
} incident_severity_t;

// Encryption algorithms
typedef enum {
    ENCRYPTION_NONE = 0,
    ENCRYPTION_AES_128 = 1,
    ENCRYPTION_AES_256 = 2,
    ENCRYPTION_RSA_2048 = 3,
    ENCRYPTION_RSA_4096 = 4,
    ENCRYPTION_ECC_P256 = 5,
    ENCRYPTION_ECC_P384 = 6,
    ENCRYPTION_CHACHA20 = 7,
    ENCRYPTION_BLOWFISH = 8,
    ENCRYPTION_3DES = 9
} encryption_algorithm_t;

// Enterprise security policy
typedef struct enterprise_security_policy {
    uint32_t policy_id;                           // Unique policy identifier
    char name[64];                                // Policy name
    char description[256];                        // Policy description
    compliance_framework_t frameworks[10];        // Applicable compliance frameworks
    uint32_t framework_count;                     // Number of frameworks
    security_zone_t zone;                         // Security zone
    threat_level_t threat_level;                  // Current threat level
    
    // Access control
    bool require_mfa;                             // Multi-factor authentication required
    bool require_encryption;                      // Data encryption required
    bool require_audit_logging;                   // Audit logging required
    uint32_t session_timeout;                     // Session timeout (minutes)
    uint32_t max_login_attempts;                  // Maximum login attempts
    uint32_t password_min_length;                 // Minimum password length
    bool require_complex_password;                // Complex password required
    uint32_t password_expiry_days;                // Password expiry (days)
    
    // Data protection
    bool data_classification_required;            // Data classification required
    bool data_encryption_at_rest;                 // Encryption at rest
    bool data_encryption_in_transit;              // Encryption in transit
    bool data_backup_required;                    // Data backup required
    uint32_t backup_retention_days;               // Backup retention (days)
    bool data_anonymization_required;             // Data anonymization required
    
    // Network security
    bool firewall_required;                       // Firewall required
    bool vpn_required;                            // VPN required
    bool network_segmentation;                    // Network segmentation
    bool intrusion_detection;                     // Intrusion detection
    bool ddos_protection;                         // DDoS protection
    
    // Application security
    bool code_review_required;                    // Code review required
    bool penetration_testing;                     // Penetration testing
    bool vulnerability_scanning;                  // Vulnerability scanning
    bool secure_development;                      // Secure development practices
    bool api_security;                            // API security
    
    // Monitoring and response
    bool real_time_monitoring;                    // Real-time monitoring
    bool incident_response;                       // Incident response
    bool threat_intelligence;                     // Threat intelligence
    bool security_metrics;                        // Security metrics
    
    // Compliance
    bool compliance_reporting;                    // Compliance reporting
    bool regular_audits;                          // Regular audits
    bool risk_assessment;                         // Risk assessment
    bool business_continuity;                     // Business continuity
    
    void* policy_data;                            // Policy-specific data
} enterprise_security_policy_t;

// Compliance framework
typedef struct compliance_framework {
    uint32_t framework_id;                        // Unique framework identifier
    compliance_framework_t type;                  // Framework type
    char name[64];                                // Framework name
    char description[256];                        // Framework description
    char version[32];                             // Framework version
    bool enabled;                                 // Framework enabled
    uint64_t last_assessment;                     // Last assessment time
    uint64_t next_assessment;                     // Next assessment time
    float compliance_score;                       // Compliance score (0-100)
    uint32_t total_controls;                      // Total controls
    uint32_t compliant_controls;                  // Compliant controls
    uint32_t non_compliant_controls;              // Non-compliant controls
    uint32_t partial_controls;                    // Partially compliant controls
    void* framework_data;                         // Framework-specific data
} compliance_framework_t;

// Audit record
typedef struct audit_record {
    uint32_t record_id;                           // Unique record identifier
    uint64_t timestamp;                           // Timestamp
    uint32_t user_id;                             // User ID
    char username[64];                            // Username
    char action[128];                             // Action performed
    char resource[256];                           // Resource accessed
    char details[512];                            // Additional details
    bool success;                                 // Action successful
    uint32_t session_id;                          // Session ID
    char ip_address[16];                          // IP address
    char user_agent[256];                         // User agent
    security_zone_t zone;                         // Security zone
    threat_level_t threat_level;                  // Threat level
    void* record_data;                            // Record-specific data
} audit_record_t;

// Encryption key
typedef struct encryption_key {
    uint32_t key_id;                              // Unique key identifier
    char name[64];                                // Key name
    char description[256];                        // Key description
    encryption_algorithm_t algorithm;             // Encryption algorithm
    uint32_t key_size;                            // Key size (bits)
    uint8_t key_data[512];                        // Key data
    uint32_t key_data_size;                       // Key data size
    uint64_t created_time;                        // Creation time
    uint64_t expiry_time;                         // Expiry time
    bool active;                                  // Key active
    bool compromised;                             // Key compromised
    uint32_t usage_count;                         // Usage count
    void* key_data_ptr;                           // Key data pointer
} encryption_key_t;

// Digital certificate
typedef struct digital_certificate {
    uint32_t cert_id;                             // Unique certificate identifier
    char name[64];                                // Certificate name
    char subject[256];                            // Subject
    char issuer[256];                             // Issuer
    char serial_number[64];                       // Serial number
    uint64_t valid_from;                          // Valid from
    uint64_t valid_until;                         // Valid until
    bool active;                                  // Certificate active
    bool revoked;                                 // Certificate revoked
    uint32_t key_size;                            // Key size
    encryption_algorithm_t algorithm;             // Signature algorithm
    uint8_t cert_data[2048];                      // Certificate data
    uint32_t cert_data_size;                      // Certificate data size
    void* cert_data_ptr;                          // Certificate data pointer
} digital_certificate_t;

// Threat indicator
typedef struct threat_indicator {
    uint32_t indicator_id;                        // Unique indicator identifier
    char name[64];                                // Indicator name
    char description[256];                        // Description
    char ioc_type[32];                            // IOC type (IP, domain, hash, etc.)
    char ioc_value[256];                          // IOC value
    threat_level_t threat_level;                  // Threat level
    uint64_t first_seen;                          // First seen
    uint64_t last_seen;                           // Last seen
    uint32_t occurrence_count;                    // Occurrence count
    bool active;                                  // Indicator active
    bool blocked;                                 // Indicator blocked
    char source[64];                              // Threat intelligence source
    float confidence;                             // Confidence score (0-100)
    void* indicator_data;                         // Indicator-specific data
} threat_indicator_t;

// Security incident
typedef struct security_incident {
    uint32_t incident_id;                         // Unique incident identifier
    char title[128];                              // Incident title
    char description[512];                        // Incident description
    incident_severity_t severity;                 // Incident severity
    threat_level_t threat_level;                  // Threat level
    uint64_t detection_time;                      // Detection time
    uint64_t resolution_time;                     // Resolution time
    uint32_t affected_users;                      // Number of affected users
    uint32_t affected_systems;                    // Number of affected systems
    char root_cause[256];                         // Root cause
    char resolution[512];                         // Resolution
    bool resolved;                                // Incident resolved
    bool reported;                                // Incident reported
    uint32_t assigned_to;                         // Assigned to user ID
    char status[32];                              // Incident status
    void* incident_data;                          // Incident-specific data
} security_incident_t;

// Enterprise security system
typedef struct enterprise_security_system {
    spinlock_t lock;                              // System lock
    bool initialized;                             // Initialization flag
    
    // Policy management
    enterprise_security_policy_t policies[MAX_SECURITY_POLICIES]; // Security policies
    uint32_t policy_count;                        // Number of policies
    
    // Compliance management
    compliance_framework_t frameworks[MAX_COMPLIANCE_FRAMEWORKS]; // Compliance frameworks
    uint32_t framework_count;                     // Number of frameworks
    
    // Audit management
    audit_record_t audit_records[MAX_AUDIT_RECORDS]; // Audit records
    uint32_t audit_record_count;                  // Number of audit records
    uint32_t audit_record_index;                  // Current audit record index
    
    // Encryption management
    encryption_key_t encryption_keys[MAX_ENCRYPTION_KEYS]; // Encryption keys
    uint32_t key_count;                           // Number of keys
    
    // Certificate management
    digital_certificate_t certificates[MAX_CERTIFICATES]; // Digital certificates
    uint32_t certificate_count;                   // Number of certificates
    
    // Threat management
    threat_indicator_t threat_indicators[MAX_THREAT_INDICATORS]; // Threat indicators
    uint32_t indicator_count;                     // Number of indicators
    
    // Incident management
    security_incident_t incidents[MAX_INCIDENT_RECORDS]; // Security incidents
    uint32_t incident_count;                      // Number of incidents
    
    // System configuration
    bool enterprise_mode;                         // Enterprise mode enabled
    bool compliance_mode;                         // Compliance mode enabled
    bool audit_mode;                              // Audit mode enabled
    bool encryption_mode;                         // Encryption mode enabled
    bool threat_detection_mode;                   // Threat detection mode enabled
    bool incident_response_mode;                  // Incident response mode enabled
    
    // System statistics
    uint64_t total_audit_records;                 // Total audit records
    uint64_t total_security_incidents;            // Total security incidents
    uint64_t total_threat_indicators;             // Total threat indicators
    uint64_t last_update;                         // Last update time
} enterprise_security_system_t;

// Enterprise security statistics
typedef struct enterprise_security_stats {
    // Policy statistics
    uint32_t total_policies;                      // Total policies
    uint32_t active_policies;                     // Active policies
    uint32_t compliant_policies;                  // Compliant policies
    uint32_t non_compliant_policies;              // Non-compliant policies
    
    // Compliance statistics
    uint32_t total_frameworks;                    // Total frameworks
    uint32_t active_frameworks;                   // Active frameworks
    float average_compliance_score;               // Average compliance score
    uint32_t total_controls;                      // Total controls
    uint32_t compliant_controls;                  // Compliant controls
    
    // Audit statistics
    uint64_t total_audit_records;                 // Total audit records
    uint64_t successful_actions;                  // Successful actions
    uint64_t failed_actions;                      // Failed actions
    uint64_t security_violations;                 // Security violations
    
    // Threat statistics
    uint32_t total_indicators;                    // Total threat indicators
    uint32_t active_indicators;                   // Active indicators
    uint32_t blocked_indicators;                  // Blocked indicators
    uint32_t high_threat_indicators;              // High threat indicators
    
    // Incident statistics
    uint32_t total_incidents;                     // Total incidents
    uint32_t open_incidents;                      // Open incidents
    uint32_t resolved_incidents;                  // Resolved incidents
    uint32_t critical_incidents;                  // Critical incidents
    
    // System statistics
    uint64_t last_update;                         // Last update time
} enterprise_security_stats_t;

// Function declarations

// System initialization
int enterprise_security_init(void);
void enterprise_security_shutdown(void);
enterprise_security_system_t* enterprise_security_get_system(void);

// Policy management
enterprise_security_policy_t* security_policy_create(const char* name, const char* description);
int security_policy_destroy(uint32_t policy_id);
int security_policy_update(uint32_t policy_id, enterprise_security_policy_t* policy);
int security_policy_apply(uint32_t policy_id, uint32_t user_id);
enterprise_security_policy_t* security_policy_find(uint32_t policy_id);
enterprise_security_policy_t* security_policy_find_by_name(const char* name);

// Compliance management
compliance_framework_t* compliance_framework_add(compliance_framework_t type, const char* name);
int compliance_framework_remove(uint32_t framework_id);
int compliance_framework_assess(uint32_t framework_id);
int compliance_framework_report(uint32_t framework_id);
compliance_framework_t* compliance_framework_find(uint32_t framework_id);
compliance_framework_t* compliance_framework_find_by_type(compliance_framework_t type);

// Audit management
int audit_record_create(uint32_t user_id, const char* action, const char* resource);
int audit_record_add_details(uint32_t record_id, const char* details);
int audit_record_set_success(uint32_t record_id, bool success);
audit_record_t* audit_record_find(uint32_t record_id);
int audit_record_search(uint32_t user_id, uint64_t start_time, uint64_t end_time, audit_record_t* results, uint32_t max_results);

// Encryption management
encryption_key_t* encryption_key_create(const char* name, encryption_algorithm_t algorithm, uint32_t key_size);
int encryption_key_destroy(uint32_t key_id);
int encryption_key_rotate(uint32_t key_id);
int encryption_key_compromise(uint32_t key_id);
encryption_key_t* encryption_key_find(uint32_t key_id);
encryption_key_t* encryption_key_find_by_name(const char* name);

// Certificate management
digital_certificate_t* certificate_create(const char* name, const char* subject, const char* issuer);
int certificate_destroy(uint32_t cert_id);
int certificate_revoke(uint32_t cert_id);
int certificate_validate(uint32_t cert_id);
digital_certificate_t* certificate_find(uint32_t cert_id);
digital_certificate_t* certificate_find_by_name(const char* name);

// Threat management
threat_indicator_t* threat_indicator_add(const char* name, const char* ioc_type, const char* ioc_value);
int threat_indicator_remove(uint32_t indicator_id);
int threat_indicator_block(uint32_t indicator_id);
int threat_indicator_update(uint32_t indicator_id, threat_level_t threat_level);
threat_indicator_t* threat_indicator_find(uint32_t indicator_id);
threat_indicator_t* threat_indicator_find_by_ioc(const char* ioc_type, const char* ioc_value);

// Incident management
security_incident_t* security_incident_create(const char* title, const char* description, incident_severity_t severity);
int security_incident_update(uint32_t incident_id, security_incident_t* incident);
int security_incident_resolve(uint32_t incident_id, const char* resolution);
int security_incident_assign(uint32_t incident_id, uint32_t user_id);
security_incident_t* security_incident_find(uint32_t incident_id);
security_incident_t* security_incident_find_by_status(const char* status);

// System configuration
int enterprise_security_enable_enterprise_mode(bool enabled);
int enterprise_security_enable_compliance_mode(bool enabled);
int enterprise_security_enable_audit_mode(bool enabled);
int enterprise_security_enable_encryption_mode(bool enabled);
int enterprise_security_enable_threat_detection(bool enabled);
int enterprise_security_enable_incident_response(bool enabled);

// Monitoring and statistics
void enterprise_security_get_stats(enterprise_security_stats_t* stats);
void enterprise_security_reset_stats(void);
int enterprise_security_perform_compliance_audit(void);
int enterprise_security_generate_security_report(void);

// Enterprise security debugging
void enterprise_security_dump_policies(void);
void enterprise_security_dump_frameworks(void);
void enterprise_security_dump_audit_records(void);
void enterprise_security_dump_encryption_keys(void);
void enterprise_security_dump_certificates(void);
void enterprise_security_dump_threat_indicators(void);
void enterprise_security_dump_incidents(void);
void enterprise_security_dump_stats(void);

#endif // ENTERPRISE_SECURITY_H 