# Rip_switch

```
# 创建vlan
int vlan 10
ip address 192.168.2.1 255.255.255.0

# 给端口g0/1绑定vlan 10
int g0/1
switchport pvid 10

# 开启rip实例
router rip 10

# 将其设置到vlan中
int vlan 10
ip rip 10 enableaa
```

