# coresim

Coresim is C code of NS3 which simulate the core network topology and assess it using Iperf.

USAGE
DCE NS3 installation is assumed had been installed.
Clone this repository to your myscript folder (YOUR_ROOT_DCE/source/ns-3-dce/myscripts)

Sample command (execute it in YOUR_ROOT_DCE/source/ns-3-dce/)
./waf --run "coresim --TypeOfConnection=p --ModeOperation=true --tcp_mem_user=4096,87380,8388608 --tcp_mem_user_wmem=4096,87380,8388608 --tcp_mem_user_rmem=4096,87380,8388608 --tcp_mem_server=4096,87380,8388608 --tcp_mem_server_wmem=4096,87380,8388608 --tcp_mem_server_rmem=4096,87380,8388608 --tcp_cc=reno --SimuTime=20.00 --user_bw_up=1Gbps --user_bw_down=10Mbps"
