#ifndef EMAIL_CLIENT_H
#define EMAIL_CLIENT_H

#include "../../gui/rendering/glass_compositor.h"
#include "../../gui/window/glass_window_manager.h"
#include <stdint.h>
#include <stdbool.h>

// Email Client - Modern email client for RaeenOS
// Provides security, encryption, and advanced email features

// Email protocols
typedef enum {
    EMAIL_PROTOCOL_POP3 = 0,     // POP3
    EMAIL_PROTOCOL_IMAP,         // IMAP
    EMAIL_PROTOCOL_SMTP,         // SMTP
    EMAIL_PROTOCOL_EXCHANGE,     // Microsoft Exchange
    EMAIL_PROTOCOL_EWS,          // Exchange Web Services
} email_protocol_t;

// Email security
typedef enum {
    EMAIL_SECURITY_NONE = 0,     // No encryption
    EMAIL_SECURITY_SSL,          // SSL/TLS
    EMAIL_SECURITY_STARTTLS,     // STARTTLS
    EMAIL_SECURITY_SMTPS,        // SMTPS
    EMAIL_SECURITY_IMAPS,        // IMAPS
    EMAIL_SECURITY_POP3S,        // POP3S
} email_security_t;

// Email encryption
typedef enum {
    EMAIL_ENCRYPTION_NONE = 0,   // No encryption
    EMAIL_ENCRYPTION_PGP,        // PGP/GPG
    EMAIL_ENCRYPTION_SMIME,      // S/MIME
    EMAIL_ENCRYPTION_END_TO_END, // End-to-end encryption
} email_encryption_t;

// Email priority
typedef enum {
    EMAIL_PRIORITY_LOW = 0,      // Low priority
    EMAIL_PRIORITY_NORMAL,       // Normal priority
    EMAIL_PRIORITY_HIGH,         // High priority
    EMAIL_PRIORITY_URGENT,       // Urgent priority
} email_priority_t;

// Email status
typedef enum {
    EMAIL_STATUS_DRAFT = 0,      // Draft
    EMAIL_STATUS_SENT,           // Sent
    EMAIL_STATUS_DELIVERED,      // Delivered
    EMAIL_STATUS_READ,           // Read
    EMAIL_STATUS_REPLIED,        // Replied
    EMAIL_STATUS_FORWARDED,      // Forwarded
    EMAIL_STATUS_DELETED,        // Deleted
    EMAIL_STATUS_SPAM,           // Spam
    EMAIL_STATUS_ARCHIVED,       // Archived
} email_status_t;

// Email attachment
typedef struct {
    uint32_t id;
    char filename[256];          // Attachment filename
    char mime_type[64];          // MIME type
    uint64_t size;               // File size in bytes
    char path[512];              // File path
    bool is_inline;              // Is inline attachment
    bool is_encrypted;           // Is encrypted
    void* data;                  // Attachment data
    raeen_texture_t* thumbnail;  // Thumbnail for images
} email_attachment_t;

// Email address
typedef struct {
    char name[128];              // Display name
    char email[256];             // Email address
    char domain[128];            // Domain part
    bool is_valid;               // Is valid email
} email_address_t;

// Email message
typedef struct {
    uint32_t id;
    char subject[512];           // Email subject
    char body[8192];             // Email body
    email_address_t from;        // Sender
    email_address_t* to;         // Recipients
    uint32_t to_count;           // Number of recipients
    email_address_t* cc;         // CC recipients
    uint32_t cc_count;           // Number of CC recipients
    email_address_t* bcc;        // BCC recipients
    uint32_t bcc_count;          // Number of BCC recipients
    email_address_t* reply_to;   // Reply-to addresses
    uint32_t reply_to_count;     // Number of reply-to addresses
    email_attachment_t* attachments; // Attachments
    uint32_t attachment_count;   // Number of attachments
    email_priority_t priority;   // Email priority
    email_status_t status;       // Email status
    email_encryption_t encryption; // Encryption type
    uint64_t date_sent;          // Sent date
    uint64_t date_received;      // Received date
    uint64_t date_read;          // Read date
    char message_id[256];        // Message ID
    char thread_id[256];         // Thread ID
    char folder[64];             // Folder
    bool is_read;                // Is read
    bool is_starred;             // Is starred
    bool is_flagged;             // Is flagged
    bool is_encrypted;           // Is encrypted
    bool is_signed;              // Is digitally signed
    void* user_data;             // User-specific data
} email_message_t;

// Email folder
typedef struct {
    uint32_t id;
    char name[64];               // Folder name
    char path[256];              // Folder path
    uint32_t message_count;      // Number of messages
    uint32_t unread_count;       // Number of unread messages
    bool is_system;              // Is system folder
    bool is_expanded;            // Is expanded in UI
    bool is_selected;            // Is selected
    email_message_t* messages;   // Messages in folder
    uint32_t message_array_size; // Size of message array
} email_folder_t;

// Email account
typedef struct {
    uint32_t id;
    char name[128];              // Account name
    char email[256];             // Email address
    char username[128];          // Username
    char password[256];          // Password (encrypted)
    char server_in[256];         // Incoming server
    char server_out[256];        // Outgoing server
    uint16_t port_in;            // Incoming port
    uint16_t port_out;           // Outgoing port
    email_protocol_t protocol_in; // Incoming protocol
    email_protocol_t protocol_out; // Outgoing protocol
    email_security_t security_in; // Incoming security
    email_security_t security_out; // Outgoing security
    bool is_active;              // Is active account
    bool is_default;             // Is default account
    bool auto_sync;              // Auto sync enabled
    uint32_t sync_interval;      // Sync interval in minutes
    uint32_t max_messages;       // Maximum messages to keep
    void* account_data;          // Account-specific data
} email_account_t;

// Email filter
typedef struct {
    uint32_t id;
    char name[64];               // Filter name
    char condition[256];         // Filter condition
    char action[64];             // Filter action
    bool is_enabled;             // Is enabled
    bool is_system;              // Is system filter
    uint32_t priority;           // Filter priority
} email_filter_t;

// Email signature
typedef struct {
    uint32_t id;
    char name[64];               // Signature name
    char content[1024];          // Signature content
    bool is_html;                // Is HTML signature
    bool is_default;             // Is default signature
    bool auto_attach;            // Auto attach to new emails
} email_signature_t;

// Email client configuration
typedef struct {
    uint32_t window_width, window_height;
    bool enable_encryption;
    bool enable_digital_signatures;
    bool enable_auto_sync;
    bool enable_notifications;
    bool enable_spam_filtering;
    bool enable_virus_scanning;
    bool enable_auto_reply;
    bool enable_offline_mode;
    uint32_t max_accounts;
    uint32_t max_messages_per_account;
    uint32_t max_attachments_per_email;
    uint32_t max_attachment_size_mb;
    uint32_t sync_interval_minutes;
    uint32_t auto_save_interval_seconds;
    char default_signature[1024];
} email_client_config_t;

// Email client context
typedef struct {
    email_client_config_t config;
    glass_compositor_t* compositor;
    glass_window_manager_t* window_manager;
    uint32_t window_id;
    email_account_t* accounts;
    uint32_t account_count;
    uint32_t max_accounts;
    email_account_t* active_account;
    email_folder_t* folders;
    uint32_t folder_count;
    uint32_t max_folders;
    email_message_t* messages;
    uint32_t message_count;
    uint32_t max_messages;
    email_message_t* selected_messages;
    uint32_t selected_count;
    uint32_t max_selected;
    email_filter_t* filters;
    uint32_t filter_count;
    uint32_t max_filters;
    email_signature_t* signatures;
    uint32_t signature_count;
    uint32_t max_signatures;
    bool initialized;
    bool is_syncing;
    bool is_offline;
    uint32_t next_message_id;
    uint32_t next_account_id;
    uint32_t next_folder_id;
    uint64_t last_sync_time;
} email_client_t;

// Function prototypes

// Initialization and shutdown
email_client_t* email_client_init(glass_compositor_t* compositor,
                                 glass_window_manager_t* window_manager,
                                 email_client_config_t* config);
void email_client_shutdown(email_client_t* client);
bool email_client_is_initialized(email_client_t* client);

// Window management
void email_client_show(email_client_t* client);
void email_client_hide(email_client_t* client);
void email_client_minimize(email_client_t* client);
void email_client_maximize(email_client_t* client);
void email_client_restore(email_client_t* client);
void email_client_close(email_client_t* client);

// Account management
uint32_t email_client_add_account(email_client_t* client, email_account_t* account);
void email_client_remove_account(email_client_t* client, uint32_t account_id);
void email_client_edit_account(email_client_t* client, uint32_t account_id, email_account_t* account);
email_account_t* email_client_get_account(email_client_t* client, uint32_t account_id);
email_account_t* email_client_get_active_account(email_client_t* client);
void email_client_set_active_account(email_client_t* client, uint32_t account_id);
uint32_t email_client_get_account_count(email_client_t* client);
bool email_client_test_account(email_client_t* client, email_account_t* account);

// Message management
uint32_t email_client_create_message(email_client_t* client);
void email_client_delete_message(email_client_t* client, uint32_t message_id);
void email_client_move_message(email_client_t* client, uint32_t message_id, const char* folder);
void email_client_copy_message(email_client_t* client, uint32_t message_id, const char* folder);
email_message_t* email_client_get_message(email_client_t* client, uint32_t message_id);
email_message_t* email_client_get_messages(email_client_t* client, uint32_t* count);
void email_client_mark_as_read(email_client_t* client, uint32_t message_id, bool read);
void email_client_mark_as_starred(email_client_t* client, uint32_t message_id, bool starred);
void email_client_mark_as_flagged(email_client_t* client, uint32_t message_id, bool flagged);

// Email composition
void email_client_set_subject(email_client_t* client, uint32_t message_id, const char* subject);
void email_client_set_body(email_client_t* client, uint32_t message_id, const char* body);
void email_client_add_recipient(email_client_t* client, uint32_t message_id, const char* name, const char* email);
void email_client_add_cc(email_client_t* client, uint32_t message_id, const char* name, const char* email);
void email_client_add_bcc(email_client_t* client, uint32_t message_id, const char* name, const char* email);
void email_client_remove_recipient(email_client_t* client, uint32_t message_id, const char* email);
void email_client_set_priority(email_client_t* client, uint32_t message_id, email_priority_t priority);
void email_client_set_encryption(email_client_t* client, uint32_t message_id, email_encryption_t encryption);

// Attachment management
uint32_t email_client_add_attachment(email_client_t* client, uint32_t message_id, const char* file_path);
void email_client_remove_attachment(email_client_t* client, uint32_t message_id, uint32_t attachment_id);
email_attachment_t* email_client_get_attachments(email_client_t* client, uint32_t message_id, uint32_t* count);
bool email_client_save_attachment(email_client_t* client, uint32_t message_id, uint32_t attachment_id, const char* path);
raeen_texture_t* email_client_get_attachment_thumbnail(email_client_t* client, uint32_t message_id, uint32_t attachment_id);

// Email sending and receiving
bool email_client_send_message(email_client_t* client, uint32_t message_id);
bool email_client_send_message_encrypted(email_client_t* client, uint32_t message_id, const char* password);
bool email_client_send_message_signed(email_client_t* client, uint32_t message_id, const char* key_path);
bool email_client_send_message_encrypted_signed(email_client_t* client, uint32_t message_id, const char* password, const char* key_path);
bool email_client_receive_messages(email_client_t* client, uint32_t account_id);
bool email_client_sync_account(email_client_t* client, uint32_t account_id);
bool email_client_sync_all_accounts(email_client_t* client);

// Folder management
uint32_t email_client_create_folder(email_client_t* client, const char* name);
void email_client_delete_folder(email_client_t* client, uint32_t folder_id);
void email_client_rename_folder(email_client_t* client, uint32_t folder_id, const char* new_name);
email_folder_t* email_client_get_folder(email_client_t* client, uint32_t folder_id);
email_folder_t* email_client_get_folders(email_client_t* client, uint32_t* count);
email_message_t* email_client_get_messages_in_folder(email_client_t* client, uint32_t folder_id, uint32_t* count);
uint32_t email_client_get_unread_count(email_client_t* client, uint32_t folder_id);

// Search functionality
email_message_t* email_client_search_messages(email_client_t* client, const char* query, uint32_t* count);
email_message_t* email_client_search_by_sender(email_client_t* client, const char* sender, uint32_t* count);
email_message_t* email_client_search_by_subject(email_client_t* client, const char* subject, uint32_t* count);
email_message_t* email_client_search_by_date(email_client_t* client, uint64_t from_date, uint64_t to_date, uint32_t* count);
email_message_t* email_client_search_attachments(email_client_t* client, const char* filename, uint32_t* count);

// Filter management
uint32_t email_client_add_filter(email_client_t* client, email_filter_t* filter);
void email_client_remove_filter(email_client_t* client, uint32_t filter_id);
void email_client_edit_filter(email_client_t* client, uint32_t filter_id, email_filter_t* filter);
email_filter_t* email_client_get_filters(email_client_t* client, uint32_t* count);
void email_client_enable_filter(email_client_t* client, uint32_t filter_id, bool enable);
void email_client_apply_filters(email_client_t* client);

// Signature management
uint32_t email_client_add_signature(email_client_t* client, email_signature_t* signature);
void email_client_remove_signature(email_client_t* client, uint32_t signature_id);
void email_client_edit_signature(email_client_t* client, uint32_t signature_id, email_signature_t* signature);
email_signature_t* email_client_get_signatures(email_client_t* client, uint32_t* count);
email_signature_t* email_client_get_default_signature(email_client_t* client);
void email_client_set_default_signature(email_client_t* client, uint32_t signature_id);

// Email actions
void email_client_reply_to_message(email_client_t* client, uint32_t message_id);
void email_client_reply_all_to_message(email_client_t* client, uint32_t message_id);
void email_client_forward_message(email_client_t* client, uint32_t message_id);
void email_client_archive_message(email_client_t* client, uint32_t message_id);
void email_client_mark_as_spam(email_client_t* client, uint32_t message_id);
void email_client_mark_as_not_spam(email_client_t* client, uint32_t message_id);
void email_client_delete_message_permanently(email_client_t* client, uint32_t message_id);

// Security and encryption
bool email_client_encrypt_message(email_client_t* client, uint32_t message_id, const char* password);
bool email_client_decrypt_message(email_client_t* client, uint32_t message_id, const char* password);
bool email_client_sign_message(email_client_t* client, uint32_t message_id, const char* key_path);
bool email_client_verify_signature(email_client_t* client, uint32_t message_id);
bool email_client_import_key(email_client_t* client, const char* key_path);
bool email_client_export_key(email_client_t* client, const char* key_path);
bool email_client_generate_key_pair(email_client_t* client, const char* name, const char* email, const char* password);

// Spam and virus protection
void email_client_enable_spam_filtering(email_client_t* client, bool enable);
bool email_client_is_spam_filtering_enabled(email_client_t* client);
void email_client_enable_virus_scanning(email_client_t* client, bool enable);
bool email_client_is_virus_scanning_enabled(email_client_t* client);
void email_client_add_spam_rule(email_client_t* client, const char* rule);
void email_client_remove_spam_rule(email_client_t* client, const char* rule);
bool email_client_scan_message_for_viruses(email_client_t* client, uint32_t message_id);

// Auto-reply and rules
void email_client_set_auto_reply(email_client_t* client, const char* message, uint64_t from_date, uint64_t to_date);
void email_client_disable_auto_reply(email_client_t* client);
bool email_client_is_auto_reply_enabled(email_client_t* client);
void email_client_add_rule(email_client_t* client, const char* condition, const char* action);
void email_client_remove_rule(email_client_t* client, uint32_t rule_id);
void email_client_apply_rules(email_client_t* client);

// Offline mode
void email_client_enable_offline_mode(email_client_t* client, bool enable);
bool email_client_is_offline_mode_enabled(email_client_t* client);
void email_client_sync_offline_messages(email_client_t* client);
uint32_t email_client_get_offline_message_count(email_client_t* client);

// Notifications
void email_client_enable_notifications(email_client_t* client, bool enable);
bool email_client_are_notifications_enabled(email_client_t* client);
void email_client_show_notification(email_client_t* client, const char* title, const char* message);
void email_client_set_notification_sound(email_client_t* client, const char* sound_path);

// Rendering
void email_client_render(email_client_t* client);
void email_client_render_message_list(email_client_t* client);
void email_client_render_message_view(email_client_t* client);
void email_client_render_compose_window(email_client_t* client);
void email_client_render_folder_tree(email_client_t* client);
void email_client_render_toolbar(email_client_t* client);

// Input handling
bool email_client_handle_mouse_move(email_client_t* client, float x, float y);
bool email_client_handle_mouse_click(email_client_t* client, float x, float y, bool left_click);
bool email_client_handle_mouse_double_click(email_client_t* client, float x, float y);
bool email_client_handle_mouse_wheel(email_client_t* client, float x, float y, float delta);
bool email_client_handle_key_press(email_client_t* client, uint32_t key_code);
bool email_client_handle_text_input(email_client_t* client, const char* text);

// Information
uint32_t email_client_get_total_message_count(email_client_t* client);
uint32_t email_client_get_unread_message_count(email_client_t* client);
uint32_t email_client_get_sent_message_count(email_client_t* client);
uint32_t email_client_get_draft_message_count(email_client_t* client);
uint64_t email_client_get_last_sync_time(email_client_t* client);
bool email_client_is_syncing(email_client_t* client);

// Utility functions
bool email_client_is_email_valid(const char* email);
char* email_client_extract_domain(const char* email);
char* email_client_format_email_address(const char* name, const char* email);
uint64_t email_client_parse_date(const char* date_string);
char* email_client_format_date(uint64_t timestamp);
char* email_client_encode_subject(const char* subject);
char* email_client_decode_subject(const char* subject);

// Callbacks
typedef void (*message_received_callback_t)(email_client_t* client, uint32_t message_id, void* user_data);
typedef void (*message_sent_callback_t)(email_client_t* client, uint32_t message_id, bool success, void* user_data);
typedef void (*sync_complete_callback_t)(email_client_t* client, uint32_t account_id, bool success, void* user_data);

void email_client_set_message_received_callback(email_client_t* client, message_received_callback_t callback, void* user_data);
void email_client_set_message_sent_callback(email_client_t* client, message_sent_callback_t callback, void* user_data);
void email_client_set_sync_complete_callback(email_client_t* client, sync_complete_callback_t callback, void* user_data);

// Preset configurations
email_client_config_t email_client_preset_normal_style(void);
email_client_config_t email_client_preset_security_style(void);
email_client_config_t email_client_preset_enterprise_style(void);

// Error handling
typedef enum {
    EMAIL_CLIENT_SUCCESS = 0,
    EMAIL_CLIENT_ERROR_INVALID_CONTEXT,
    EMAIL_CLIENT_ERROR_INVALID_ACCOUNT,
    EMAIL_CLIENT_ERROR_INVALID_MESSAGE,
    EMAIL_CLIENT_ERROR_NETWORK_FAILED,
    EMAIL_CLIENT_ERROR_AUTHENTICATION_FAILED,
    EMAIL_CLIENT_ERROR_ENCRYPTION_FAILED,
    EMAIL_CLIENT_ERROR_OUT_OF_MEMORY,
    EMAIL_CLIENT_ERROR_INVALID_EMAIL,
    EMAIL_CLIENT_ERROR_TIMEOUT,
} email_client_error_t;

email_client_error_t email_client_get_last_error(void);
const char* email_client_get_error_string(email_client_error_t error);

#endif // EMAIL_CLIENT_H 