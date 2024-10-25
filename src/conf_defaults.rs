#[macro_export]
macro_rules! conf_defaults {
    () => {
        [
            ("web_dir".to_string(), "./example/".to_string()),
            ("web_addr".to_string(), "127.0.0.1".to_string()),
            ("web_port".to_string(), "8080".to_string()),
            ("dns_addr".to_string(), "127.0.0.1".to_string()),
            ("dns_port".to_string(), "8053".to_string()),
            ("dhcp_addr".to_string(), "127.0.0.1".to_string()),
            ("dhcp_port".to_string(), "8067".to_string()),
        ]
    };
}
