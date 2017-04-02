Consider two fbs files:
`root1.fbs`:
```
namespace warchant;

table Account {
  id: uint;
  name: string;
}

root_type Account;
```
`non-root1.fbs`:
```
include "root1.fbs";

namespace warchant;

table Transaction{
  account: Account;
}
```

 `Get{{STRUCT_NAME}}` is generated twice in commit 2aec880347394c04862dbbba3a1c09740e3a2198: 
 - in `root1.fbs`
 - in `non-root1.fbs`

 But, since `non-root1.fbs` includes `root1.fbs`, it causes to redefinition of `Get{{STRUCT_NAME}}`:
 ```
 #include "root1_generated.h"
 ...
 ```

