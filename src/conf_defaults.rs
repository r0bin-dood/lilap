#[macro_export]
macro_rules! conf_defaults {
    () => {
        [
            ("link_addr".to_string(), "127.0.0.1".to_string()),
            ("link_parent_iface".to_string(), "wlo1".to_string()),
            ("link_iface".to_string(), "lilap0".to_string()),
            ("link_ssid".to_string(), "lilapFree".to_string()),
            ("web_dir".to_string(), "./example/".to_string()),
            ("web_port".to_string(), "80".to_string()),
            ("dns_port".to_string(), "53".to_string()),
            ("dhcp_src_port".to_string(), "67".to_string()),
            ("dhcp_dst_port".to_string(), "68".to_string()),
        ]
    };
}
