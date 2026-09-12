# Architecture validation matrix

first column caller, first raw callee

|            | hal | sysCore | sysCall | interfaces | services | tasks |
| ---        | --- | ---     | ---     | ---        | ---      | ---   |
| hal        | Y   | N       |  N      | Y          | N        | N     |
| sysCore    | Y   | Y       |  N      | Y          | N        | N     |
| sysCall    | Y   | Y       |  Y      | Y          | N        | N     |
| interfaces | N   | N       |  N      | Y          | N        | N     |
| services   | N   | N       |  Y      | N          | Y        | N     |
| tasks      | N   | N       |  Y      | N          | Y        | Y     |
