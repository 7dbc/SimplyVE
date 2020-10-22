package model

import (
    log "github.com/cihub/seelog"
)

type Traffic struct {
    HostName      string `json:"hostname"`
    Adapter       string `json:"adapter"`
    TXBytes       int64  `json:"tx_bytes"`
    RXBytes       int64  `json:"rx_bytes"`
    PreTXBytes    int64  `json:"pre_tx_bytes"`
    PreRXBytes    int64  `json:"pre_rx_bytes"`
}


func check(e error) {
    if e != nil {
        log.Errorf("error!! %+v", e)
        panic(e)
    }
}
