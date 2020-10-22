package model

type ConfInfo struct {
    counter    int64
    SSHUser    string `json:"sshuser"`
    SSHPwd     string `json:"sshpwd"`
    LocalProxy     string   `json:"localproxy"`
    Connection struct {
        Collector    string `json:"collector"`
        Interval     int    `json:"interval"`
    } `json:"connection"`
    GeneralIntervals int `json:"generalintervals"`
    Common []string `json:"common"`
    Hosts  []struct {
        Name                string   `json:"name"`
        IP                  string   `json:"ip"`
        Port                string   `json:"port"`
        Networkconnectivity []string `json:"networkConnectivity"`
        Commands            []CommandStruct `json:"commands"`
    } `json:"hosts"`
}

type CommandStruct struct {
    Name         string `json:"name"`
    Comandstring string `json:"comandString"`
    Intervals    int    `json:"intervals"`
}
