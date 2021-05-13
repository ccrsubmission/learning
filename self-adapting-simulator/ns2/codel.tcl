# Codel test script v120513

# Run this to run CoDel AQM tests.
# ns codel.tcl f w c {b}Mb s d r
# where:
#   f = # ftps
#   w = # PackMime connections per second
#   c = # CBRs
#   b = bottleneck bandwidth in Mbps
#   s = filesize for ftp, -1 for infinite
#   d = dynamic bandwidth, if non-zero, changes (kind of kludgey)
#       have to set the specific change ratios in this file (below)
#   r = number of "reverse" ftps

set ns [new Simulator]

# common params
set stopTime 300
set seed 0

# These are defaults if values not set on command line

set num_ftps 1
set ftp_start_time 0
set web_rate 0
set web_start_time 0
set revftp 0
#
# cbr
set num_cbrs 0
set cbr_rate 0.064Mb
set cbr_packet_size 100

set bottleneck 10Mb
#
#for a 10MB ftp
set filesize 10000000
set dynamic_bw 0
set greedy 0


set learning_algo "ucb.json"
set interval_selector_command {
    new IntervalSelector/Fixed [delay_parse 100ms] 0 1
}
set reward_command { 
    # TODO: 1.0 is not a proper scale here, see codel.py
    new Reward/Throughput [delay_parse 10ms] [delay_parse 10ms] 1.0
}

# Parse command line

set arg 0
set learning_start_time 0

if {$argc >= [expr $arg+1]} {
    set stopTime [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set trace_dir [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set trace_nam [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set seed [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set num_ftps [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set ftp_start_time [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set filesize [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set greedy [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set revftp [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set web_rate [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set web_start_time [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set num_cbrs [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set cbr_rate [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set cbr_packet_size [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set bottleneck [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set dynamic_bw [lindex $argv $arg]
    incr arg 1
}

if {$argc >= [expr $arg+1]} {
    set accessdly [lindex $argv $arg]
    incr arg 1
}

if {$argc >= [expr $arg+1]} {
    set bdelay [lindex $argv $arg]
    incr arg 1
}


if {$argc >= [expr $arg+1]} {
    set learning_start_time [lindex $argv $arg]
    incr arg 1
}
if {$argc >= [expr $arg+1]} {
    set learning_algo [lindex $argv $arg]
    incr arg 1
}

if {$argc >= [expr $arg+1]} {
    set interval_selector_command [lindex $argv $arg]
    incr arg 1
}

if {$argc >= [expr $arg+1]} {
    set reward_command [lindex $argv $arg]
    incr arg 1
}

# CoDel values
for {} {$arg < $argc} {incr arg 1} { 
    lappend queue_management_algos [lindex $argv $arg]
}

set bw [bw_parse $bottleneck]
if { $revftp >= 1} {
    set num_revs $revftp
} else {
    set num_revs 0
}

# experiment settings
set psize 1500
if { $bw < 1000000 } { set psize 500 }
set nominal_rtt [delay_parse 100ms]
set realrtt [expr 2*(2*$accessdly + $bdelay)]
#
# puts "accessdly $accessdly bneckdly $bdelay realrtt $realrtt bneckbw $bw"

global defaultRNG
if {$seed != 0} {
    $defaultRNG seed $seed
    ns-random [expr $seed+1]
} else {
    $defaultRNG seed 0
    ns-random 0
}

# ------- config info is all above this line ----------

#bdp in packets, based on the nominal rtt
set bdp [expr round($bw*$nominal_rtt/(8*$psize))]

Trace set show_tcphdr_ 1
set startTime 0.0

#TCP parameters - have to set both for FTPs and PackMime

Agent/TCP set window_ [expr $bdp*16]
Agent/TCP set segsize_ [expr $psize-40]
Agent/TCP set packetSize_ [expr $psize-40]
Agent/TCP set windowInit_ 4
Agent/TCP set segsperack_ 1
Agent/TCP set timestamps_ true
set delack 0.4
Agent/TCP set interval_ $delack

Agent/TCP/FullTcp set window_ [expr $bdp*16]
Agent/TCP/FullTcp set segsize_ [expr $psize-40]
Agent/TCP/FullTcp set packetSize_ [expr $psize-40]
Agent/TCP/FullTcp set windowInit_ 4
Agent/TCP/FullTcp set segsperack_ 1
Agent/TCP/FullTcp set timestamps_ true
Agent/TCP/FullTcp set interval_ $delack


Agent/TCP/Linux instproc done {} {
    global ns filesize
#this doesn't seem to work, had to hack tcp-linux.cc to do repeat ftps
    $self set closed_ 0
#needs to be delayed by at least .3sec to slow start
    puts "[$ns now] TCP/Linux proc done called"
    $ns at [expr [$ns now] + 0.3] "$self send $filesize"
}

# problem is that idle() in tcp.cc never seems to get called...


Application/FTP instproc resume {} {
puts "called resume"
        global filesize
        $self send $filesize
#   $ns at [expr [$ns now] + 0.5] "[$self agent] reset"
    $ns at [expr [$ns now] + 0.5] "[$self agent] send $filesize"
}

Application/FTP instproc fire {} {
        global filesize
        $self instvar maxpkts_
        set maxpkts_ $filesize
    [$self agent] set maxpkts_ $filesize
    $self send $maxpkts_
    puts "fire() FTP"
}

set web_tcp_trace [open $trace_dir/web_tcp.tr w]

PackMimeHTTP instproc alloc-tcp {tcptype} {
    global web_tcp_trace

    if {$tcptype != "Reno"} {
        set tcp [new Agent/TCP/FullTcp/$tcptype]
    } else {
        set tcp [new Agent/TCP/FullTcp]
    }

    $tcp attach-stats $web_tcp_trace
    return $tcp
}

Class Application/FTPWithStats -superclass Application/FTP

Application/FTPWithStats instproc stop {duration} {
    global psize
    [$self agent] stats $duration [[$self agent] set fid_] $psize

    eval $self next
}

PackMimeHTTP instproc done {tcp} {
    global psize
    # print stats 
    $tcp stats 1.0 [$tcp set fid_] $psize

    # the connection is done, so recycle the agent
    $self recycle $tcp
}

#buffersizes
set buffersize [expr $bdp]
set buffersize1 [expr $bdp*10]

#set Flow_id 1

proc setup_forward_link { link } {
    global ns learning_algo target interval interval_selector_command
    global learning_start_time 
    global bw reward_command trace_dir queue_management_algos

    set codel_drop_trace [open $trace_dir/codel_drop.tr w]
    set codel_trace [open $trace_dir/codel.tr w]
    for {set k 0} {$k < [llength $queue_management_algos]} {incr k 1} { 
        set codel [eval [lindex $queue_management_algos $k]]
        $link add_policy $codel
    }

    $link set_learning $learning_algo
    $link set_interval_selector [eval $interval_selector_command]
    $link set_reward [eval $reward_command]

    $ns at $learning_start_time "$link start_learning"


    set learning_trace_channel [open $trace_dir/learning.tr w]
    $link attach $learning_trace_channel
    set reward_trace_channel [open $trace_dir/reward.tr w]
    $link attach-reward $reward_trace_channel
}

proc setup_backward_link { link } {
    global target interval
    set codel [new Queue/CoDel]
    $codel set target_ 0.005
    $codel set interval_ 0.1
    $link add_policy $codel
}

proc build_topology { ns } {
    # nodes n0 and n1 are the server and client side gateways and
    # the link between them is the congested slow link. n0 -> n1
    # handles all the server to client traffic.
    #
    # if the web_rate is non-zero, node n2 will be the packmime server cloud
    # and node n3 will be the client cloud.
    #
    # num_ftps server nodes and client nodes are created for the ftp sessions.
    # the first client node is n{2+w} and the first server node is n{2+f+w}
    # where 'f' is num_ftps and 'w' is 1 if web_rate>0 and 0 otherwise.
    # servers will be even numbered nodes, clients odd
    # Warning: the numbering here is ridiculously complicated

    global bw bdelay accessdly buffersize buffersize1 filesize node_cnt trace_dir
    global learning_num_subintervals
    set node_cnt 2

    # congested link
    global n0 n1
    set n0 [$ns node]
    set n1 [$ns node]
    $ns duplex-link $n0 $n1 $bw ${bdelay}ms Learning
    $ns duplex-link-op $n0 $n1 orient right
    $ns duplex-link-op $n0 $n1 queuePos 0.5
    $ns duplex-link-op $n1 $n0 queuePos 1.5
    $ns queue-limit $n0 $n1 $buffersize
    $ns queue-limit $n1 $n0 $buffersize
    set node_cnt 2

    #dynamic bandwidth
    # these are the multipliers for changing bw, times initial set bw
    # edit these values to get different patterns
    global stopTime dynamic_bw
    array names bw_changes
    set bw_changes(1) 0.1
    set bw_changes(2) 0.01
    set bw_changes(3) 0.5
    set bw_changes(4) 0.01
    set bw_changes(5) 1.0

    # puts "bottleneck starts at [[[$ns link $n0 $n1] link] set bandwidth_]bps"
    for {set k 1} {$k <= $dynamic_bw} {incr k 1} {
        set changeTime [expr $k*$stopTime/($dynamic_bw+1)]
        set f $bw_changes($k)
        set newBW [expr $f*$bw]
        puts "change at $changeTime to [expr $newBW/1000000.]Mbps"
        $ns at $changeTime "[[$ns link $n0 $n1] link] set bandwidth_ $newBW"
        $ns at $changeTime "[[$ns link $n1 $n0] link] set bandwidth_ $newBW"
        $ns at $changeTime "puts $newBW"
    }

    set li_10 [[$ns link $n1 $n0] queue]
    set li_01 [[$ns link $n0 $n1] queue]

    setup_forward_link $li_01
    setup_backward_link $li_10

    global num_ftps web_rate num_cbrs greedy num_revs
    set linkbw [expr $bw*10]

    set w [expr $web_rate > 0]
    if {$w} {
        global n2 n3
    #server
        set n2 [$ns node]
        $ns duplex-link $n2 $n0 $linkbw ${accessdly}ms DropTail
        $ns queue-limit $n2 $n0 $buffersize1
        $ns queue-limit $n0 $n2 $buffersize1

    #client
        set n3 [$ns node]
        $ns duplex-link $n1 $n3 $linkbw ${accessdly}ms DropTail
        $ns queue-limit $n1 $n3 $buffersize1
        $ns queue-limit $n3 $n1 $buffersize1
        set node_cnt 4
    }
#need to fix the angles if use nam
    for {set k 0} {$k < $num_ftps} {incr k 1} {
        # servers
        set j $node_cnt
        global n$j
        set n$j [$ns node]
        if {$greedy > 0 && $k == 0} {
            $ns duplex-link [set n$j] $n0 $linkbw 1ms DropTail
        } else {
            $ns duplex-link [set n$j] $n0 $linkbw ${accessdly}ms DropTail
        }
        $ns queue-limit [set n$j] $n0 $buffersize1
        $ns queue-limit $n0 [set n$j] $buffersize1
        set angle [expr $num_ftps>1? 0.75+($k-1)*.5/($num_ftps-1) : 1]
        $ns duplex-link-op $n0 [set n$j] orient $angle

        incr node_cnt

        # clients
        set j $node_cnt
        global n$j
        set n$j [$ns node]
        set dly [expr ${accessdly} +($k+1)]
        $ns duplex-link $n1 [set n$j] $linkbw  ${dly}ms  DropTail
        $ns queue-limit $n1 [set n$j] $buffersize1
        $ns queue-limit [set n$j] $n1 $buffersize1
        set angle [expr $num_ftps>1? fmod(2.25-($k-1)*.5/($num_ftps-1), 2) : 0]
        $ns duplex-link-op $n1 [set n$j] orient $angle

        incr node_cnt
    }
    for {set k 0} {$k < $num_cbrs} {incr k 1} {
        # servers
        set j $node_cnt
        global n$j
        set n$j [$ns node]
        $ns duplex-link [set n$j] $n0 $linkbw ${accessdly}ms DropTail
        $ns queue-limit [set n$j] $n0 $buffersize1
        $ns queue-limit $n0 [set n$j] $buffersize1
        set angle [expr $num_cbrs>1? 0.75+($k-1)*.5/($num_cbrs-1) : 1]
        $ns duplex-link-op $n0 [set n$j] orient $angle

        incr node_cnt

        # clients
        set j $node_cnt
        global n$j
        set n$j [$ns node]
        $ns duplex-link $n1 [set n$j] $linkbw  ${accessdly}ms  DropTail
        $ns queue-limit $n1 [set n$j] $buffersize1
        $ns queue-limit [set n$j] $n1 $buffersize1
        set angle [expr $num_cbrs>1? fmod(2.25-($k-1)*.5/($num_cbrs-1), 2) : 0]
        $ns duplex-link-op $n1 [set n$j] orient $angle
        
        incr node_cnt
    }
#reverse direction ftps
    for {set k 0} {$k < $num_revs} {incr k 1} {
        # clients
        set j $node_cnt
        global n$j
        set n$j [$ns node]
        $ns duplex-link [set n$j] $n0 $linkbw ${accessdly}ms DropTail
        $ns queue-limit [set n$j] $n0 $buffersize1
        $ns queue-limit $n0 [set n$j] $buffersize1
        set angle [expr $num_ftps>1? 0.75+($k-1)*.5/($num_ftps-1) : 1]
        $ns duplex-link-op $n0 [set n$j] orient $angle
    incr node_cnt

        # servers
        set j $node_cnt
        global n$j
        set n$j [$ns node]
        set dly [expr ($accessdly)*1.1 +($k+1)]
        $ns duplex-link $n1 [set n$j] $linkbw  ${dly}ms  DropTail
        $ns queue-limit $n1 [set n$j] $buffersize1
        $ns queue-limit [set n$j] $n1 $buffersize1
        set angle [expr $num_ftps>1? fmod(2.25-($k-1)*.5/($num_ftps-1), 2) : 0]
        $ns duplex-link-op $n1 [set n$j] orient $angle
    incr node_cnt
    }
}

proc build_cbr {cnd snd startTime timeToStop Flow_id} {
    global ns cbr_rate cbr_packet_size
    set udp [$ns create-connection UDP $snd LossMonitor $cnd $Flow_id]
    set cbr [new Application/Traffic/CBR]
    $cbr attach-agent $udp
    # change these for different types of CBRs
    $cbr set packetSize_ $cbr_packet_size
    $cbr set rate_ $cbr_rate
    $ns at $startTime "$cbr start"
    $ns at $timeToStop "$cbr stop"
}

set ftp_tcp_trace [open $trace_dir/ftp_tcp.tr w]

# cnd is client node, snd is server node
proc build_ftpclient {cnd snd startTime timeToStop Flow_id tcp_trace} {
    global ns filesize greedy revftp trace_dir ftp_tcp_trace
    set ctcp [$ns create-connection TCP/Linux $snd TCPSink/Sack1 $cnd $Flow_id]
    $ctcp select_ca cubic
    $ctcp attach-stats $ftp_tcp_trace
    set ftp [$ctcp attach-app FTPWithStats]
    $ftp set enableResume_ true
    $ftp set type_ FTP 

    # enable tracing
    $ctcp trace cwnd_
    $ctcp attach $tcp_trace

#set up a single infinite ftp with smallest RTT
    if {$greedy > 0 || $filesize < 0} {
     $ns at $startTime "$ftp start"
     set greedy 0
    } else {
     $ns at $startTime "$ftp send $filesize"
    }
    $ns at $timeToStop "$ftp stop [expr $timeToStop-$startTime]"
}

proc build_webs {cnd snd rate startTime timeToStop} {
    set CLIENT 0
    set SERVER 1

    # SETUP PACKMIME
    set pm [new PackMimeHTTP]
    $pm set-TCP Sack
    $pm set-client $cnd
    $pm set-server $snd
    $pm set-rate $rate;                    # new connections per second
    $pm set-http-1.1;                      # use HTTP/1.1

    # create RandomVariables
    set flow_arrive [new RandomVariable/PackMimeHTTPFlowArrive $rate]
    set req_size [new RandomVariable/PackMimeHTTPFileSize $rate $CLIENT]
    set rsp_size [new RandomVariable/PackMimeHTTPFileSize $rate $SERVER]

    # assign RNGs to RandomVariables
    $flow_arrive use-rng [new RNG]
    $req_size use-rng [new RNG]
    $rsp_size use-rng [new RNG]

    # set PackMime variables
    $pm set-flow_arrive $flow_arrive
    $pm set-req_size $req_size
    $pm set-rsp_size $rsp_size

    # enable tracing (to calculate FCT)
    global trace_dir
    $pm set-outfile "$trace_dir/web.tr"

    global ns web_start_time
    $ns at [expr $startTime+$web_start_time] "$pm start"
    $ns at $timeToStop "$pm stop"
}

proc uniform {a b} {
    expr $a + (($b- $a) * ([ns-random]*1.0/0x7fffffff))
}


proc finish {} {
    global ns trace_dir

    if { $trace_dir != "none" } {
        global qm delay
        set f [open $trace_dir/stats a]
        puts $f "[$qm set bdrops_] [$qm set bdepartures_] [$delay mean] [$delay variance]"
    }

    $ns halt
    $ns flush-trace
    exit 0
}

if { $trace_dir != "none" && $trace_nam != 0 } {
    # enabling NAM
    $ns color 2 blue
    $ns color 3 red
    $ns color 4 yellow
    $ns color 5 green
    $ns namtrace-all [open $trace_dir/all.nam w]
}

build_topology $ns

if { $trace_dir != "none"} {
    # basic stat tracing
    set delay [new Samples]
    set qm [$ns monitor-queue $n0 $n1 0]
    $qm set-delay-samples $delay

    # queue tracing
    $ns trace-queue $n0 $n1 [open $trace_dir/queue.tr w]
}

#reverse direction
#$ns trace-queue $n1 $n0 [open /tmp/$fname w]

set node_cnt 2
if {$web_rate > 0} {
    build_webs $n3 $n2 $web_rate 0 $stopTime
    set node_cnt 4
}

set tcp_trace [open $trace_dir/tcp.tr w]

for {set k 1} {$k <= $num_ftps} {incr k 1} {
    set j $node_cnt
    incr node_cnt
    set i $node_cnt
    build_ftpclient [set n$i] [set n$j]  \
       [expr $ftp_start_time+$startTime+1.0*($k-1)+[uniform 0.0 100.0]] $stopTime $i $tcp_trace
#        $startTime $stopTime $i $tcp_trace\
#       [expr 1.0*($k-1)] $stopTime $i $tcp_trace
    incr node_cnt
}

for {set k 1} {$k <= $num_cbrs} {incr k 1} {
    set j $node_cnt
    incr node_cnt
    set i $node_cnt
    build_cbr [set n$i] [set n$j]  \
        [expr $startTime+($k-1)*[uniform 0.0 2.0]] $stopTime $i
    incr node_cnt
}

#for reverse direction, give client smaller number
for {set k 1} {$k <= $num_revs} {incr k 1} {
    set j $node_cnt
    incr node_cnt
    set i $node_cnt
    build_ftpclient [set n$j] [set n$i] $startTime $stopTime $j $tcp_trace
    incr node_cnt
}

$ns at [expr $stopTime ] "finish"
$ns run

exit 0
