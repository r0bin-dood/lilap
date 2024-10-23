use crate::server::*;
use crate::server::web::*;

impl HasStateField for Web {
    fn state(&self) -> &ServerState {
        &self.state
    }
    fn state_mut(&mut self) -> &mut ServerState {
        &mut self.state
    }
}