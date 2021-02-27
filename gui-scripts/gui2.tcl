#!/usr/bin/wish
#
# Define the global environment, everything lives under $HOME/speckle-control
# Change SPECKLE_DIR to move the code somewhere else
#
## \file gui2.tcl
# \brief This contains helper routines to process header items
#
# This Source Code Form is subject to the terms of the GNU Public\n
# License, v. 2 If a copy of the GPL was not distributed with this file,\n
# You can obtain one at https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html\n
#\n
# Copyright(c) 2018 The Random Factory (www.randomfactory.com) \n
#\n
#
#
#\code
## Documented proc \c debuglog .
# \param[in] msg Text of debug message
#
#  Output a debug message to the log file. Log files are saved in the /tmp
#  directory with names like /tmp/speckle_12345678.log
#
#
# Globals :\n
#		FLOG - File handle of open log file
#
proc debuglog { msg } {
global FLOG
   puts $FLOG $msg
   flush $FLOG
}



## Documented proc \c plotTimings .
#
#  Plot the timing infornation stored in a binary FITS extension\n
#  of an image cube from a kinetic series observation.\n
#  The times are stored as TAI, but we subtract the initial time\n
#  before plotting. Gnuplot is used to plot.
#
# Globals :\n
#		SCOPE - Array of telescope information\n
#		env - Environment variables
#
proc plotTimings { } {
global env SCOPE
   set it [tk_getOpenFile -initialdir $SCOPE(datadir)]
   if { [file exists $it] } {
      set fh [fits open $it]
      $fh move +1
      set times [$fh get table]
      set fout [open /tmp/timings w]
      set start [lindex $times 0]
      foreach i $times {
          set t [expr $i-$start]
          if { [expr abs($t)] < 1000 } {puts $fout $t}
      }
      close $fout
      fits close $fh
      exec echo "plot \"/tmp/timings\"" | gnuplot -p
   }
}


## Documented proc \c setfitsbits .
# \param[in] type FITS data type
#
# Set the FITS data type for subsequent images
#
proc setfitsbits { type } {
   commandAndor red "fitsbits $type"   
   commandAndor blue "fitsbits $type"
}

## Documented proc \c dataquality .
# \param[in] type Type of data quality information , image, cloud, water, background
# \param[in] value Value of data quality
#
# Set the data quality values for headers
#
#
# Globals :\n
#		DATAQUAL - Array of data qualities\n
#		DATAQUALT - Array of data quality types\n
#		TELEMETRY - Array of telemetry for header and database usage
#
proc dataquality { type value } {
global DATAQUAL DATAQUALT TELEMETRY
   set DATAQUAL($type) $value
   set TELEMETRY(tcs.weather.$type) $value
   .main.[set type] configure -text "DQ $DATAQUALT($type) : $value"
}


## Documented proc \c speckleGuiMode .
# \param[in] mode Selected mode, observing or engineering
#
# Set the GUI mode geometry , observing or engineering
#
#
# Globals :\n
#		SPECKLE - Array of GUI proerties
#
proc speckleGuiMode { mode } {
global SPECKLE
  wm geometry . $SPECKLE($mode)
}

## Documented proc \c validInteger .
# \param[in] win Widget id
# \param[in] event Type of event
# \param[in] X New value
# \param[in] oldX Previous value
# \param[in] min Minimum allowed value
# \param[in] max Maximum allowed value
#
# Check if input field is a valid integer
#
#
proc validInteger {win event X oldX min max} {
        # Make sure min<=max
        if {$min > $max} {
            set tmp $min; set min $max; set max $tmp
        }
        # Allow valid integers, empty strings, sign without number
        # Reject Octal numbers, but allow a single "0"
        # Which signes are allowed ?
        if {($min <= 0) && ($max >= 0)} {   ;# positive & negative sign
            set pattern {^[+-]?(()|0|([1-9][0-9]*))$}
        } elseif {$max < 0} {               ;# negative sign
            set pattern {^[-]?(()|0|([1-9][0-9]*))$}
        } else {                            ;# positive sign
            set pattern {^[+]?(()|0|([1-9][0-9]*))$}
        }
        # Weak integer checking: allow empty string, empty sign, reject octals
        set weakCheck [regexp $pattern $X]
        # if weak check fails, continue with old value
        if {! $weakCheck} {set X $oldX}
        # Strong integer checking with range
        set strongCheck [expr {[string is int -strict $X] && ($X >= $min) && ($X <= $max)}]

        switch $event {
            key {
                $win configure -bg [expr {$strongCheck ? "white" : "yellow"}]
                return $weakCheck
            }
            focusout {
                if {! $strongCheck} {$win configure -bg red}
                return $strongCheck
            }
            default {
                return 1
            }
        }
} 


## Documented proc \c validFloat .
# \param[in] win Widget id
# \param[in] event Type of event
# \param[in] X New value
# \param[in] oldX Previous value
#
# Check if input field is a valid floating point number
#
#
proc validFloat {win event X oldX} {
        set strongCheck [expr {[string is double $X]}]
        if {! $strongCheck} {set X $oldX}
        switch $event {
            key {
                $win configure -bg [expr {$strongCheck ? "white" : "yellow"}]
                return $strongCheck
            }
            focusout {
                if {! $strongCheck} {$win configure -bg red}
                return $strongCheck
            }
            default {
                return 1
            }
        }
} 


## Documented proc \c setKineticMode .
# 
# Set Kinetics mode for camera
#
#
# Globals :\n
#		ANDOR_CFG - Array of camera parameters\n
#		ACQREGION - Region of interest parameters
#
proc setKineticMode { } {
global ANDOR_CFG ACQREGION
  if { $ANDOR_CFG(kineticMode) }  {
    if { $ACQREGION(geom) != 1024 } {
      commandAndor red "setframe roi"
      commandAndor blue "setframe roi"
      set LASTACQ roi
     } else {
      commandAndor red  "setframe fullkinetic"
      commandAndor blue "setframe fullkinetic"
    }
  } else {
    commandAndor red "setframe fullframe"
    commandAndor blue "setframe fullframe"
  }
}

## Documented proc \c setDisplayMode .
# 
# Set FFT display mode
#
#
# Globals :\n
#		ANDOR_CFG - Array of camera parameters\n
#
proc setDisplayMode  { } {
global ANDOR_CFG
   andorSetControl 0 showfft $ANDOR_CFG(showfft)
}

## Documented proc \c restartads9 .
# \param[in] arm Red or Blue
# \param[in] geom Display geometry
# 
# Restart a ds9 instance
#
#
proc restartads9  { arm geom } {
  exec ds9[set arm] -geometry $geom &
  after 3000 commandAndor $arm connectds9
}


## Documented proc \c setBinning .
# 
# Set image binning parameters for cameras
#
#
# Globals :\n
#		ANDOR_CFG - Array of camera parameters\n
#
proc setBinning { } {
global ANDOR_CFG
   commandAndor red "setbinning $ANDOR_CFG(binning) $ANDOR_CFG(binning)"
   commandAndor blue "setbinning $ANDOR_CFG(binning) $ANDOR_CFG(binning)"
}


## Documented proc \c exposureType .
# 
# Set exposure type
#
#
# Globals :\n
#		SCOPE - Array of GUI parameters\n
#
proc exposureType { etype } {
global SCOPE
   set SCOPE(exptype) $etype
   .main.exptype configure -text $etype
}


# \endcode


set SPECKLE_DIR $env(SPECKLE_DIR)
set DEBUG 1
set RAWTEMP 0
set SPECKLEGUI 1
set NOBLT 1
set where [exec ip route]
set gw [lindex $where 2]
set SCOPE(telescope) GEMINI
set SCOPE(site) GEMINI_N
set SCOPE(latitude) 19:49:00
set SCOPE(longitude) 155:28:00

if { $gw == "140.252.61.1" || $env(TELESCOPE) == "WIYN" } {
  set SCOPE(latitude) 31:57:11.78
  set SCOPE(longitude) 07:26:27.97
  set SCOPE(telescope) WIYN
  set SCOPE(site) KPNO
  set SCOPE(instrument) NESSI
  set env(GEMINISITE) NA
}
set now [clock seconds]
set FLOG [open /tmp/speckleLog_[set now].log w]
exec xterm -geometry +540+800 -e tail -f /tmp/speckleLog_[set now].log &

#
# Load the procedures
#
source $SPECKLE_DIR/gui-scripts/general.tcl
setutc
###source $SPECKLE_DIR/gui-scripts/display.tcl
###source $SPECKLE_DIR/gui-scripts/temperature.tcl
###source $SPECKLE_DIR/gui-scripts/calibration.tcl
source $SPECKLE_DIR/gui-scripts/observe.tcl
set ACQREGION(xs) 1
set ACQREGION(xe) 1024
set ACQREGION(ys) 1
set ACQREGION(ye) 1024
set LASTACQ fullframe
source $SPECKLE_DIR/andor/andor.tcl


# Create the status window. This window is used to display informative messages
# during initialization.
#

toplevel .status -width 500 -height 100
wm title .status "Speckle Instrument Control"
wm geometry .status +20+30
label .status.msg -bg LightBlue -fg Black -text Initialising -width 50 -font "Helvetica 30 bold"
pack .status.msg


#
# Define the path to the shared libraries.
# These libraries are used to add facilities to the default tcl/tk
# wish shell. 
#
set libs $SPECKLE_DIR/lib
load $libs/liboriel.so

#
# Load the tcl interface to FITS (Flexible image transport system) disk files
# FITS is the standard disk file format for Astronomical data
#

showstatus "Loading FitsTcl"
if { [file exists $libs/libfitstcl.so] } {
  load $libs/libfitstcl.so
} else {
  package require fitsTcl
}

fits version

# Prepare for Ccd image buffering package
package ifneeded ccd 1.0       [load $libs/libccd.so]

# Load packages provided by dynamically loadable libraries
showstatus "Loading CCD package"
package require ccd

lappend auto_path $libs/bwidget1.9.8
package require BWidget
proc getlocaltime { } {exec date}


set SCOPE(datadir) $env(HOME)/data

#
#  Define globals for temperature control

set TEMPS ""
set STATUS(tempgraph) 1
set FRAME 1
set STATUS(readout) 0




#
#  Update status display
#
showstatus "Building user interface"
set SCOPE(equinox) "2000.0"

#
#  Create countdown window widgets
#
set f "Helvetica -30 bold"
toplevel .countdown -bg orange -width 535 -height 115
label .countdown.lf -text "Frame # " -bg orange -font $f
label .countdown.lt -text "Seconds : " -bg orange -font $f
label .countdown.f -text "???" -bg orange -font $f
label .countdown.t -text "???" -bg orange -font $f
place .countdown.lf -x 10 -y 40
place .countdown.f -x 140 -y 40
place .countdown.lt -x 230 -y 40
place .countdown.t -x 380 -y 40
wm withdraw .countdown


#
#  Create main window and its menus
#
wm title . "Speckle Instrument Control"
frame .mbar -width 936 -height 30 -bg gray
menubutton .mbar.file -text "File" -fg black -bg gray -menu .mbar.file.m
menubutton .mbar.temp -text "Temperature" -fg black -bg gray -menu .mbar.temp.m
menubutton .mbar.help -text "Help" -fg black -bg gray -menu .mbar.help.m
menubutton .mbar.tools -text "Tools" -fg black -bg gray -menu .mbar.tools.m
pack .mbar
place .mbar.file -x 0 -y 0
place .mbar.temp -x 180 -y 0
place .mbar.tools -x 300 -y 0
place .mbar.help -x 880 -y 0
menu .mbar.file.m 
menu .mbar.temp.m
menu .mbar.tools.m
menu .mbar.help.m
#.mbar.file.m add command -label "Open" -command fileopen
.mbar.file.m add command -label "Save"  -command "savespecklegui"
.mbar.file.m add command -label "USHORT image format" -command "setfitsbits USHORT_IMG"
.mbar.file.m add command -label "ULONG  image format" -command "setfitsbits ULONG_IMG"

#.mbar.file.m add command -label "Save As" -command filesaveas
.mbar.file.m add command -label "Shutdown" -command shutdown
.mbar.temp.m add command -label "Cooler on" -command "setpoint on"
.mbar.temp.m add command -label "Cooler off" -command "setpoint off"
.mbar.temp.m add command -label "Cooler to ambient" -command  {set ok [confirmaction "Ramp temperature to ambient"] ; if {$ok} {setpoint amb}}
#.mbar.temp.m add command -label "Plot averaged temps" -command {set RAWTEMP 0}
#.mbar.temp.m add command -label "Plot raw temps" -command {set RAWTEMP 1}
.mbar.tools.m add command -label "Engineering" -command "speckleGuiMode engineeringGui"
.mbar.help.m add command -label "Users Guide" -command {exec firefox file://$SPECKLE_DIR/doc/user-guide.html &}
.mbar.help.m add command -label "Code Documentation" -command {exec firefox file://$SPECKLE_DIR/doc/code/html/index.html &}
.mbar.tools.m add command -label "Observing" -command "speckleGuiMode observingGui"
.mbar.tools.m add command -label "Filter Selection" -command "wm deiconify .filters"
.mbar.tools.m add command -label "Camera status" -command "cameraStatuses"
.mbar.tools.m add command -label "Plot timings" -command "plotTimings"
.mbar.tools.m add command -label "ds9 red"  -command "restartads9 red +1200+0"
.mbar.tools.m add command -label "ds9 blue" -command "restartads9 blue +0+0"

set FITSBITS(SHORT_IMG)    16
set FITSBITS(LONG_IMG)     32
set FITSBITS(FLOAT_IMG)   -32
set FITSBITS(USHORT_IMG)   20
set FITSBITS(ULONG_IMG)    40
### not supported
#set FITSBITS(LONGLONG_IMG) 64
#set FITSBITS(BYTE_IMG)     8 
#set FITSBITS(DOUBLE_IMG)  -64

#
#  Initialize telescope/user variables
#
frame .main -bg gray -width 936 -height 330
pack .main -side bottom
set iy 10
foreach item "target ProgID ra dec telescope instrument" {
   label .main.l$item -bg gray -fg black -text $item
   place .main.l$item -x 300 -y $iy
   entry .main.v$item -bg white -fg black -relief sunken -width 18 -textvariable SCOPE($item) -justify right
   place .main.v$item -x 400 -y $iy
   incr iy 24 
}


#
#  Create main observation management widgets
#
#
set bwkey text
set bwfont font
SpinBox .main.exposure -width 10  -range "0.0 1048.75 1" -textvariable SCOPE(exposure) -justify right -validate all -vcmd {validFloat %W %V %P %s}
place .main.exposure -x 100 -y 20

SpinBox .main.exposureRed -width 10  -range "0.0 1048.75 1" -textvariable SCOPE(exposureRed) -justify right -validate all -vcmd {validFloat %W %V %P %s}
place .main.exposureRed -x 5000 -y 20
set STATUS(exposureMode) clone


SpinBox .main.numexp -width 10   -range "1 1000 1" -textvariable SCOPE(numframes) -justify right -validate all -vcmd {validInteger %W %V %P %s 1 262000}

place .main.numexp -x 100 -y 50
set opts "Object Focus Acquire Flat SkyFlat Dark Zero PSFstandard CalBinary"
###ComboBox .main.exptype -width 10  -values "$opts" -textvariable SCOPE(exptype) -justify right
menubutton .main.exptype -width 10 -text "Object" -relief raised -fg black -bg gray80 -menu .main.exptype.m
menu .main.exptype.m
foreach etype "Object Focus Acquire Flat SkyFlat Dark Zero PSFstandard CalBinary Test"  {
  .main.exptype.m add command -label "$etype" -command "exposureType $etype"
}
set SCOPE(exptype) "Object"

SpinBox .main.numseq -width 10   -range "1 100 1" -textvariable SCOPE(numseq) -justify right -validate all -vcmd {validInteger %W %V %P %s 1 20}
place .main.numseq -x 100 -y 109
label .main.laccum -text "Accum." -bg gray
SpinBox .main.numaccum -width 7   -range "1 10000 1" -textvariable SCOPE(numaccum) -justify right -validate all -vcmd {validInteger %W %V %P %s 1 1000}
place .main.exptype -x 100 -y 80
place .main.numaccum -x 2000 -y 106
label .main.lexp -text Exposure -bg gray
label .main.lnum -text "Num. Frames" -bg gray
label .main.lseq -text "Num. Seq." -bg gray
label .main.ltyp -text "Exp. Type" -bg gray
place .main.laccum -x 2000 -y 108
place .main.lexp -x 16 -y 23
place .main.lnum -x 16 -y 53
place .main.ltyp -x 16 -y 83
place .main.lseq -x 16 -y 109
checkbutton .main.showfft -bg gray  -text "Display FFT" -variable ANDOR_CFG(showfft) -command setDisplayMode -highlightthickness 0
place .main.showfft -x 210 -y 82


label .main.lbin -text Binning  -bg gray
SpinBox .main.binning -width 4 -values "1 2 4 8 16" -textvariable ANDOR_CFG(binning) -justify right 
place .main.lbin -x 220 -y 23
place .main.binning -x 280 -y 20

exposureType Object
set SCOPE(numaccum) 1
set SCOPE(numseq) 1

button .main.seldir -width 38 -text "$SCOPE(datadir)" -command "choosedir data data"
place .main.seldir -x 20 -y 286
label .main.lname -bg gray -fg black -text "File name :"
place .main.lname -x 16 -y 135
entry .main.imagename -width 18 -bg white -fg black -textvariable SCOPE(imagename) -justify right
place .main.imagename -x 100 -y 135

label .main.bcamtemp -bg gray -fg blue -text "???.?? degC" -bg gray
place .main.bcamtemp -x 353 -y 2
label .main.rcamtemp -bg gray -fg blue -text "???.?? degC" -bg gray
place .main.rcamtemp -x 453 -y 2

menubutton .main.rois -width 20 -text "Set ROI's" -relief raised -fg black -bg gray80 -menu .main.rois.m
place .main.rois -x 20 -y 230
menu .main.rois.m
.main.rois.m add command -label "Acq-roi-64"  -command "observe region64"
.main.rois.m add command -label "Acq-roi-128" -command "observe region128"
.main.rois.m add command -label "Acq-roi-256" -command "observe region256"
.main.rois.m add command -label "Acq-roi-512" -command "observe region512"
.main.rois.m add command -label "Acq-roi-768" -command "observe region768"
.main.rois.m add command -label "Acq-full"    -command "observe regionall"
.main.rois.m add command -label "Adjust ROI"  -command "observe manual"

checkbutton .main.defcenter -bg gray  -text "Default ROI Centers" -variable ACQREGION(useDefaultCenters) -highlightthickness 0
place .main.defcenter -x 200 -y 230

entry .main.seqnum -width 6 -bg white -fg black -textvariable SCOPE(seqnum) -justify right -validate all -vcmd {validInteger %W %V %P %s 1 999999}
place .main.seqnum -x 275 -y 135
set SCOPE(seqnum) 1
button .main.observe -width 18 -height 2 -text "Observe" -bg green -command startsequence
button .main.abort -width 5 -height 2 -text "Abort" -relief sunken -bg gray -command abortsequence
place .main.observe -x 20  -y 167
place .main.abort   -x 120 -y 167

menubutton .main.rawiq -text "DQ - image" -width 21 -fg black -bg gray80 -menu .main.rawiq.m -relief raised
menu .main.rawiq.m
.main.rawiq.m add command -label "RAWIQ 20%" -command "dataquality rawiq 20"
.main.rawiq.m add command -label "RAWIQ 70%" -command "dataquality rawiq 70"
.main.rawiq.m add command -label "RAWIQ 85%" -command "dataquality rawiq 85"
.main.rawiq.m add command -label "RAWIQ ANY" -command "dataquality rawiq 0"
place .main.rawiq -x 362 -y 200

menubutton .main.rawcc -text "DQ - cloud" -width 21 -fg black -bg gray80 -menu .main.rawcc.m -relief raised
menu .main.rawcc.m
.main.rawcc.m add command -label "RAWCC 50%" -command "dataquality rawcc 50-percentile"
.main.rawcc.m add command -label "RAWCC 70%" -command "dataquality rawcc 70-percentile"
.main.rawcc.m add command -label "RAWCC 80%" -command "dataquality rawcc 80-percentile"
.main.rawcc.m add command -label "RAWCC ANY" -command "dataquality rawcc Any"
place .main.rawcc -x 362 -y 230

menubutton .main.rawwv -text "DQ - water" -width 21 -fg black -bg gray80 -menu .main.rawwv.m -relief raised
menu .main.rawwv.m
.main.rawwv.m add command -label "RAWWV 20%" -command "dataquality rawwv 20"
.main.rawwv.m add command -label "RAWWV 50%" -command "dataquality rawwv 50"
.main.rawwv.m add command -label "RAWWV 80%" -command "dataquality rawwv 80"
.main.rawwv.m add command -label "RAWWV ANY" -command "dataquality rawwv 0"
place .main.rawwv -x 362 -y 260

menubutton .main.rawbg -text "DQ - bg" -width 21 -fg black -bg gray80 -menu .main.rawbg.m -relief raised
menu .main.rawbg.m
.main.rawbg.m add command -label "RAWBG 20%" -command "dataquality rawbg 20"
.main.rawbg.m add command -label "RAWBG 50%" -command "dataquality rawbg 50"
.main.rawbg.m add command -label "RAWBG 80%" -command "dataquality rawbg 80"
.main.rawbg.m add command -label "RAWBG ANY" -command "dataquality rawbg 0"
place .main.rawbg -x 362 -y 290


foreach q "rawbg rawcc rawwv rawiq" { set DATAQUAL($q) 0 }
set DATAQUALT(rawiq) image
set DATAQUALT(rawcc) cloud
set DATAQUALT(rawwv) water
set DATAQUALT(rawbg) bg



.main.abort configure -relief sunken -fg LightGray
set SCOPE(autodisplay) 1
set SCOPE(autobias) 0
set SCOPE(autocalibrate) 0
set SCOPE(overwrite) 0
set STATUS(abort) 0
set STATUS(pause) 0
set STATUS(readout) 0



if { 0 } {
#
#  Set up the default structures for temperature control/plotting
#
set LASTTEMP 0.0
set TIMES "0"
set SETPOINTS "0.0"
set AVGTEMPS "0.0"
set i -60
set xdata ""
set ydata ""
set ysetp ""
while { $i < 0 } {
  lappend xdata $i
  lappend ydata $AVGTEMPS
  lappend ysetp $SETPOINTS
  incr i 1
}

source $SPECKLE_DIR/gui-scripts/plotchart.tcl
#set f [.p.props getframe Temperature]
toplevel .tplot -width 500 -height 220 -bg white
canvas .tplot.plot -width 500 -height 220
set TEMPWIDGET [::Plotchart::createStripchart .tplot.plot  "0 60 10" "-80 30 10"]
$TEMPWIDGET dataconfig setpoint -color yellow
$TEMPWIDGET dataconfig ccd -color blue
$TEMPWIDGET title "Temps.  (setpoint=yellow, ccd=blue)"
$TEMPWIDGET xtext Sample
$TEMPWIDGET ytext Temp.
place .tplot.plot -x 0 -y 0
wm withdraw .tplot
}

#
#  Do the actual setup of the GUI, to sync it with the camera status
#
label .main.zaberA  -bg gray -text "Zaber A @ ???? : ????"
label .main.zaberB  -bg gray -text "Zaber B @ ???? : ????"
label .main.zaberInput  -bg gray -text "Zaber Input @ ???? : ????"
if { $env(TELESCOPE) == "GEMINI" } {
  label .main.zaberFocus  -bg gray -text "Zaber Focus @ ???? : ????"
  label .main.zaberPickoff  -bg gray -text "Zaber Pickoff @ ???? : ????"
  place .main.zaberFocus -x 720 -y 200
  place .main.zaberPickoff -x 720 -y 230
}
place .main.zaberA -x 560 -y 200
place .main.zaberB -x 560 -y 230
place .main.zaberInput -x 560 -y 260



source $SPECKLE_DIR/gui-scripts/speckle_gui.tcl

label .lowlevel.lsim -text "Simulate :" -bg gray
checkbutton .lowlevel.simandor -bg gray  -text "Andors" -variable ANDORS(sim) -highlightthickness 0
checkbutton .lowlevel.simzaber -bg gray  -text "Zabers" -variable ZABERS(sim) -highlightthickness 0
checkbutton .lowlevel.simfilter -bg gray  -text "Filters" -variable FWHEELS(sim) -highlightthickness 0
checkbutton .lowlevel.simtlm -bg gray  -text "Telemetry" -variable SPKTELEM(sim) -highlightthickness 0
place .lowlevel.lsim -x 10 -y 620
place .lowlevel.simandor -x 80 -y 620
place .lowlevel.simzaber -x 150 -y 620
place .lowlevel.simfilter -x 80 -y 640
place .lowlevel.simtlm -x 150 -y 640


if { $SCOPE(telescope) == "GEMINI" } {
  checkbutton .lowlevel.simpico -bg gray  -text "Picos" -variable PICOS(sim) -highlightthickness 0
  place .lowlevel.simpico -x 80 -y 660
  set SCOPE(instrument) "Alopeke"
  if { $env(GEMINISITE) == "south" } {
    set SCOPE(instrument) "Zorro"
  }
} else {
  set SCOPE(instrument) "NESSI"
}
   

#
#
#  Call the camera setup code, and the telescope setup code
#
showstatus "Initializing cameras"
set ANDOR_SOCKET(red) 0
set ANDOR_SOCKET(blue) 0

resetSingleAndors fullframe
set STATUS(busy) 0
load $SPECKLE_DIR/lib/andorTclInit.so
andorConnectShmem2
initControl
set ANDOR_CFG(kineticMode) 1
setKineticMode 
commandAndor red "outputamp $ANDOR_EMCCD"
commandAndor blue "outputamp $ANDOR_EMCCD"

set INSTRUMENT(red,emccd) 1
set INSTRUMENT(blue,emccd) 1

set CCDID 0
set RAWTEMP 0
set REMAINING 0


#
#  Set defaults for observation parameters
#

set OBSPARS(Object) "1.0 1 1"
set OBSPARS(Focus)  "0.1 1 1"
set OBSPARS(Acquire) "1.0 1 1"
set OBSPARS(Flat)    "1.0 1 1"
set OBSPARS(Dark)    "100.0 1 0"
set OBSPARS(Zero)    "0.01 1 0"
set OBSPARS(Skyflat) "0.1 1 1"
set OBSPARS(PSFstandard)    "0.01 1 0"
set OBSPARS(CalBinary)    "0.01 1 0"

set LASTBIN(x) 1
set LASTBIN(y) 1
set SCOPE(obsdate) [exec date -u +%Y-%m-%dT%H:%M:%S.0]
set d  [split $SCOPE(obsdate) "-"]
set SCOPE(equinox) [format %7.2f [expr [lindex $d 0]+[lindex $d 1]./12.]]

###trace variable CONFIG w watchconfig
###trace variable SCOPE w watchscope


#
#  Reset to the last used configuration if available
#
set SCOPE(datadir) $env(HOME)/data
set SCOPE(darktime) 0.0
set SCOPE(numframes) 1
set SCOPE(numseq) 1

#
#  Start monitoring the temperature
#
##monitortemp
wm withdraw .status
wm geometry . +20+30
#setfullframe

focus .

#
#  Stop the user from destroying the windows by accident
#

wm protocol .countdown WM_DELETE_WINDOW {wm iconify .countdown}
wm protocol .status WM_DELETE_WINDOW {wm iconify .status}
wm protocol .       WM_DELETE_WINDOW {wm iconify .status}
wm protocol .mimicSpeckle WM_DELETE_WINDOW {wm iconify .status}
wm protocol .camerastatus WM_DELETE_WINDOW {wm iconify .status}
wm protocol .filters WM_DELETE_WINDOW {wm iconify .status}

set d [string tolower [exec date -u +%B]]
set SCOPE(imagename) "N[exec date -u +%Y%m%d]N"
set SCOPE(preamble) N

if { $env(TELESCOPE) != "WIYN" } {
   set SCOPE(preamble) N
   set SCOPE(imagename) "N[exec date -u +%Y%m%d]A"
   set SCOPE(instrument) "Alopeke"
   proc updateRedisTelemetry { {mode ""} {obs "" } } { }
   if { $env(GEMINISITE) == "south" } {
     set SCOPE(instrument) "Zorro"
     set SCOPE(preamble) S
     set SCOPE(imagename) "S[exec date -u +%Y%m%d]Z"
     set hnow [lindex [split [exec date -u] " :"] 3]
     if { $hnow > 20 || $hnow < 8} {
        set SCOPE(imagename) "S[clock format [expr  [clock seconds] + 24*3600] -format %Y%m%d -gmt 1]Z"
     }
   }
}

set ckey [string tolower $env(TELESCOPE)]
if { $env(GEMINISITE) =="south" } {
   set ckey "geminiS"
}
source $SPECKLE_DIR/andorsConfiguration.[set ckey]

catch {
    set all [lsort [glob $SCOPE(datadir)/[set SCOPE(preamble)]*.fits]]
    set last [file rootname [split [lindex $all end] _]]
    set SCOPE(seqnum) [expr [string trimleft [string range [file tail $last] 10 13] 0] + 1]
    if { $SCOPE(seqnum) == "fits" } {
       set SCOPE(seqnum) 1
    }
}


exposureType Object
exposureMode clone
zaberJogger input
speckleGuiMode observingGui
.main.exptype configure -text $SCOPE(imagetype)

debuglog "****************************************************************"
debuglog "************                                       *************"
debuglog "************  Speckle GUI initialization complete  *************"
debuglog "************                                       *************"
debuglog "****************************************************************"

source $env(SPECKLE_DIR)/gui-scripts/inventory.tcl
puts stdout  "****************************************************************"
puts stdout  "************                                       *************"
puts stdout  "************  Speckle GUI initialization complete  *************"
puts stdout  "************                                       *************"
puts stdout  "****************************************************************"





