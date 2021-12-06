# Timing Constraints Table Based on Current Command and Next Command

|1st command                                                                                                                                                                                                                                                                                                                          |2nd command|To                             |Timing Constraint Internal to these two instructions (DRAM)                                             |Notes (Page Reference to DDR4 datasheet)|
|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------|-------------------------------|--------------------------------------------------------------------------------------------------------|----------------------------------------|
|Precharge                                                                                                                                                                                                                                                                                                                            |Precharge  |Same bank group, same bank     |tRC = 76 = tRAS + tRP (how quickly after activating that you can issue and complete a precharge command)|                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|0 constraint (1 DRAM cycle) (row must have satisfied tRAS)                                              |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |0 constraint (1 DRAM cycle) (row must have satisfied tRAS)                                              |                                        |
|Precharge                                                                                                                                                                                                                                                                                                                            |Act        |Same bank group, same bank     |tRP = 24                                                                                                |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|0 constraint (1 DRAM cycle) (row must have satisfied tRAS)                                              |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |0 constraint (1 DRAM cycle) (row must have satisfied tRAS)                                              |                                        |
|Precharge                                                                                                                                                                                                                                                                                                                            |Read       |Same bank group, same bank     |NOT POSSIBLE                                                                                            |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|0 constraint (1 DRAM cycle) (row must have satisfied tRAS)                                              |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |0 constraint (1 DRAM cycle) (row must have satisfied tRAS)                                              |                                        |
|Precharge                                                                                                                                                                                                                                                                                                                            |Write      |Same bank group, same bank     |NOT POSSIBLE                                                                                            |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|0 constraint (1 DRAM cycle) (row must have satisfied tRAS)                                              |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |0 constraint (1 DRAM cycle) (row must have satisfied tRAS)                                              |                                        |
|Act                                                                                                                                                                                                                                                                                                                                  |Precharge  |Same bank group, same bank     |tRAS = 52                                                                                               |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|0 constraint (1 DRAM cycles)                                                                            |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |0 constraint (1 DRAM cycles)                                                                            |                                        |
|Act                                                                                                                                                                                                                                                                                                                                  |Act        |Same bank group, same bank     |NOT POSSIBLE                                                                                            |Page 139                                |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|tRRD_L = 6                                                                                              |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |tRRD_S = 4                                                                                              |                                        |
|Act                                                                                                                                                                                                                                                                                                                                  |Read       |Same bank group, same bank     |tRCD = 24                                                                                               |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|0 constraint (1 DRAM cycles)                                                                            |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |0 constraint (1 DRAM cycles)                                                                            |                                        |
|Act                                                                                                                                                                                                                                                                                                                                  |Write      |Same bank group, same bank     |tRCD = 24                                                                                               |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|0 constraint (1 DRAM cycles)                                                                            |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |0 constraint (1 DRAM cycles)                                                                            |                                        |
|Read                                                                                                                                                                                                                                                                                                                                 |Precharge  |Same bank group, same bank     |RTP = 12, must also satisfy time between Act and Precharge (tRAS)                                       |Page 209                                |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|RTP = 12                                                                                                |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |RTP = 12                                                                                                |                                        |
|Read                                                                                                                                                                                                                                                                                                                                 |Act        |Same bank group, same bank     |NOT POSSIBLE. Read will either lead into another read or write, or precharge                            |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|0 constraint (1 DRAM cycles)                                                                            |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |0 constraint (1 DRAM cycles)                                                                            |                                        |
|Read                                                                                                                                                                                                                                                                                                                                 |Read       |Same bank group, same bank     |tCCD_L = 8                                                                                              |Page 198                                |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|tCCD_L = 8                                                                                              |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |tCCD_S = 4                                                                                              |                                        |
|Read                                                                                                                                                                                                                                                                                                                                 |Write      |Same bank group, same bank     |tCAS + tBURST - CWL = 8                                                                                 |Page 203                                |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|tCCD_L = 8                                                                                              |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |tCCD_S + tWTR = 8                                                                                       |We invented tWTR to prevent bus conflict|
|Write                                                                                                                                                                                                                                                                                                                                |Precharge  |Same bank group, same bank     |tCWL + tBURST + tWR = 20 + 4 + 20 = 44                                                                  |Page 237                                |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|0 constraint (1 DRAM cycles)                                                                            |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |0 constraint (1 DRAM cycles)                                                                            |                                        |
|Write                                                                                                                                                                                                                                                                                                                                |Act        |Same bank group, same bank     |NOT POSSIBLE                                                                                            |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|0 constraint (1 DRAM cycles)                                                                            |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |0 constraint (1 DRAM cycles)                                                                            |                                        |
|Write                                                                                                                                                                                                                                                                                                                                |Read       |Same bank group, same bank     |tCWL + tBURST + tWTR_L = 20 + 4 + 12 = 36                                                               |Page 233                                |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|tCWL + tBURST + tWTR_L = 20 + 4 + 4 = 36                                                                |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |tCWL + tBURST + tWTR_S = 20 + 4 + 4 = 28                                                                |                                        |
|Write                                                                                                                                                                                                                                                                                                                                |Write      |Same bank group, same bank     |tCCD_L = 8                                                                                              |Page 227                                |
|                                                                                                                                                                                                                                                                                                                                     |           |Same bank group, different bank|tCCD_L = 8                                                                                              |                                        |
|                                                                                                                                                                                                                                                                                                                                     |           |Different bank group           |tCCD_S = 4                                                                                              |                                        |