#!/usr/local/bin/perl
use Socket;
use Oraperl;
$lda = &ora_login('DESY','KHVOROST','ANATOLY')
    || die $ora_errst;

$fmt = "'DD-MON-YYYY HH24:MI:SS'";
$csr_insert = &ora_open($lda,"insert into alarm (alarm_date,record_name,alarm_type_1,alarm_1,alarm_type_2,alarm_2,value) values(to_date(:1,$fmt),:2,:3,:4,:5,:6,:7)" )
    || die $ora_errst;

$this_host = 'epicsg.desy.de'; 
$port = 3999; 
$server_addr = (gethostbyname($this_host))[4]; 
$server_struct = pack("S n a4 x8", AF_INET, $port, $server_addr); 
$proto = (getprotobyname('tcp'))[2]; 
socket(SOCK, PF_INET, SOCK_STREAM, $proto)||  die "Failed to initialize socket: $!\n";

setsockopt(SOCK, SOL_SOCKET, SO_REUSEADDR,1) || die "setsockopt() failed: $!\n"; 
bind(SOCK, $server_struct) || die "bind() failed: $!\n"; 
listen(SOCK, SOMAXCONN) || die "listen() failed: $!\n";

for (;;) { 

          $remote_host = accept(NEWSOCK, SOCK); 
          die "accept() error: $!\n" unless ($remote_host);
recv(NEWSOCK,$buf,200,$server_struct);

#print $buf;

($alarm_d,$alarm_t,$x,$record_name,$alarm_type_1,$alarm_1,$alarm_type_2,$alarm_2,$value) =
$buf =~/(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/;
$alarm_date = $alarm_d . ' ' . $alarm_t;
#print "$alarm_date,$alarm_t,$record_name,$alarm_type_1,$alarm_1,$alarm_type_2,$alarm_2,$value\n";     
    
&ora_bind($csr_insert,$alarm_date,$record_name,$alarm_type_1,$alarm_1,$alarm_type_2,$alarm_2,$value)  || die $ora_errst;
&ora_commit($lda);
}





