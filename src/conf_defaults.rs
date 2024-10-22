#[macro_export]
macro_rules! conf_defaults {
    () => {
        [
            ("log_dir".to_string(), "stdout".to_string()),
            ("web_dir".to_string(), "/var/www/html/".to_string()),
            ("web_addr".to_string(), "127.0.0.1".to_string()),
            ("web_port".to_string(), "8080".to_string()),
        ]
    };
}
