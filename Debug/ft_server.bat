@echo off
net use Q: /delete /yes
net use Q: \\192.168.202.6\data1\ANX_PGM unimos@ftdata /user:chipmos\ftdata
net use X: /delete /y
net use X: \\192.168.202.166\ft_server_data unimos@ftdata /user:chipmos\ftdata
exit