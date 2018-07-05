# RelayForRouter

Send command like that: 

`curl -i -X GET 192.168.1.9/switch`

`curl -i -X GET 192.168.1.9/offThenOnSwitch`

`curl -i -X GET 192.168.1.9/temporarySwitch`

`curl -i -X GET 192.168.1.9/state`

Pinout for NodeMCU:

```
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
```
