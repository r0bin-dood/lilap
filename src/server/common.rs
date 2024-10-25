use crate::server::*;
use crate::server::web::*;
use crate::server::dns::*;
use crate::server::dhcp::*;
use crate::server::link::*;

impl HasStateField for Web {
    fn state(&self) -> &ServerState {
        &self.state
    }
    fn state_mut(&mut self) -> &mut ServerState {
        &mut self.state
    }
}

impl HasStateField for Dns {
    fn state(&self) -> &ServerState {
        &self.state
    }
    fn state_mut(&mut self) -> &mut ServerState {
        &mut self.state
    }
}

impl HasStateField for Dhcp {
    fn state(&self) -> &ServerState {
        &self.state
    }
    fn state_mut(&mut self) -> &mut ServerState {
        &mut self.state
    }
}

impl HasStateField for Link {
    fn state(&self) -> &ServerState {
        &self.state
    }
    fn state_mut(&mut self) -> &mut ServerState {
        &mut self.state
    }
}