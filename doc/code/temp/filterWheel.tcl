#!/usr/bin/tclsh
## \file filterWheel.tcl
# \brief Filter wheel control scripts
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
## Documented proc \c loadFiltersConfig .
# \param[in] fname Name of filter configuration file
#
# Load the filter wheels configurations
#
# Globals :\n
#		SPECKLE_DIR - Directory path of speckle code\n
#		FWHEELS - Array of filter wheel configuration and settings\n
#		SCOPE - Array of telescope configuration
#
proc loadFiltersConfig { fname } {
global SPECKLE_DIR FWHEELS SCOPE env
   set fname "[set fname].[string tolower $SCOPE(telescope)]"
   if { $env(GEMINISITE) == "south" } { 
      set fname "[set fname]S"
   }
   if { [file exists $SPECKLE_DIR/$fname] == 0 } {
     errordialog "Filters configuration file $SPECKLE_DIR/$fname\n does not exist"
   } else {
     source $SPECKLE_DIR/$fname
   }
}

## Documented proc \c saveFiltersConfig .
# \param[in] fname Name of filter configuration file
#
# Save the filter wheels configurations
#
# Globals :\n
#		SPECKLE_DIR - Directory path of speckle code\n
#		FWHEELS - Array of filter wheel configuration and settings\n
#		SCOPE - Array of telescope configuration
#
proc saveFiltersConfig { fname } {
global SPECKLE_DIR FWHEELS SCOPE env
   set fname "[set fname].[string tolower $SCOPE(telescope)]"
   if { $env(GEMINISITE) == "south" } { 
      set fname "[set fname]S"
   }
   set fcfg [open $SPECKLE_DIR/$fname w]
   puts $fcfg  "#!/usr/bin/tclsh
   echoFiltersConfig $fcfg
   close $fcfg
   debuglog "Saved Filters configuration in $SPECKLE_DIR/$fname"
}


## Documented proc \c logFiltersConfig .
#
# Log filter wheel configurations
#
# Globals :
#		FLOG - File handle of open log file
#
proc logFiltersConfig { } {
global FLOG
  echoFiltersConfig $FLOG
}

## Documented proc \c logFiltersConfig .
# \param[in] fcfg - File handle
#
# Log filter wheel configurations
#
# Globals :
#		FWHEELS - Array of filter wheel configuration and settings
#
proc echoFiltersConfig { {fcfg stdout} } {
global FWHEELS
   puts $fcfg  "# Filters stage configuration parameters
"
   foreach i "red blue" {
     foreach p "port 1 2 3 4 5 6" {
         puts $fcfg "set FWHEELS($i,$p) \"$FWHEELS($i,$p)\""
     }
     puts $fcfg ""
   }
}

## Documented proc \c selectfilter .
# \param[in] id Wheel identifier
# \param[in] n Filter position
#
#  Move filter to selected position
#
# Globals :
#		FWHEELS - Array of filter wheel configuration and settings
#
proc selectfilter { id n } {
global FILTERWHEEL FWHEELS MAXFILTERS
  set i 1
  while { $i <= $MAXFILTERS } {
     .filters.f$id$i configure -relief raised -bg gray -activebackground gray
     incr i 1
  }
  .filters.f$id$n configure -bg yellow -activebackground yellow
  if { $id == "red" } {
     catch {.lowlevel.rfilter configure -bg yellow -activebackground yellow}
  } else {
     catch {.lowlevel.bfilter configure -bg yellow -activebackground yellow}
  }
  update
  set result [setOrielFilter $FWHEELS($id,handle) $n]
  set msg [split $result \n]
  if { [string range [lindex $msg 3] 0 9] == "USB error:" } {
    set it [ tk_dialog .d "SPECKLE Filter Wheel $id" "$result" {} -1 OK]
  } else {
    .filters.f$id$n configure -bg green -relief sunken -activebackground green
     if { $id == "red" } {
        catch {.lowlevel.rfilter configure -bg green -activebackground green}
     } else {
        catch {.lowlevel.bfilter configure -bg green -activebackground green}
    }
    mimicMode $id filter $FWHEELS($id,$n)
    if { $id == "red" } {
      catch {.lowlevel.rfilter configure -text "Filter = $FWHEELS(red,$n)"}
    }
    if { $id == "blue" } {
      catch {.lowlevel.bfilter configure -text "Filter = $FWHEELS(blue,$n)"}
    }
    set FWHEELS($id,position) $n
    if { [info proc adjustTeleFocus] != "" } {
       set delta  [expr $FWHEELS($id,$FWHEELS($id,position),focus) - $FWHEELS(focusoffset)]
       adjustTeleFocus $delta
       set FWHEELS(focusoffset) $FWHEELS($id,$FWHEELS($id,position),focus)
    }
  }
}

## Documented proc \c findWheels .
#
#  Lookup filter wheels by serial number
#
# Globals :
#		FWSERIAL - Filter wheel serial numbers
#
proc findWheels { } {
global FWSERIAL
   set fw [split [exec lsusb] \n]
   set id 1
   foreach i $fw {
      if { [lindex $i 6] == "Newport" } {
        catch {
          set iusb($id) [string trim [lindex $i 1]:[lindex $i 3] :]
          set info [exec lsusb -v -s $iusb($id)]
          set FWSERIAL($iusb($id)) [lindex $info [expr [lsearch $info iSerial] +2]]
          incr id 1
        }
      }
   }
}


## Documented proc \c filterWheelHelp .
#
#  Print help menu for optional implementation as a service
#
proc filterWheelHelp { }  {
  puts stdout "Filter Wheel commands :
tcl	Command         Function
open			Open a connection
close			Close connection
getID	IDN?		Query to determine filter wheel model and firmware revision.
setPos	FILT X		Sets the active filter position to value X (1-6)
getPos	FILT?		Query to determine the active filter position (1-6)
setFlt	FLTX ID		Set the  filter type/lable in position X
getFlt	FLTX ID?	Query to determine the  filter type/label in position X
next	NEXT		Increments the filter position clockwise by 1 position
prev	PREV		Increments the filter position counter clockwise by 1 position	
manual	MBTN (Y/N)	Sets the manual push button mode to enable (Y) or disable (N)
status	STB?		Query to determine the status of the unit
reset	RST		Resets the controller to default settings & operation
clear	CLS		Clears any status error messages
setHS	HSHK X		Set the unit handshake mode
getHS	HSHK?		Query to determine the hanshake mode of the unit
dotest	TST		Initiates a system self test
getTest	TST?		Query to determine the self test status/result
"
}

## Documented proc \c fwService .
#
#  Command parser stub for optional implementation as a service
#
proc fwService { arm op {p1 ""} {p2 ""} {p3 ""} } {
global FWHEEL
   switch $op {
       open    {}
       close   {}
       getID   {}
       setPos  {}
       getPos  {}
       setFlt  {}
       getFlt  {}
       next    {}
       prev    {}
       manual  {}
       status  {}
       reset   {}
       clear   {}
       setHS   {}
       getHS   {}
       dotest  {}
       getTest {}
  }
}

## Documented proc \c resetFilterWheel .
# \param[in] id Wheel identifier
#
#  Reset filter wheel
#
proc resetFilterWheel { id } {
  catch {
   oriel_write_cmd $id RST
   after 7000
   set res [oriel_read_result $id]
   oriel_write_cmd $id FILT?
   after 500
   set res [oriel_read_result $id]
   return $res
  }
}

## Documented proc \c setOrielFilter .
# \param[in] id Wheel identifier
# \param[in] n Filter position
#
#  Call low level code to move filter to selected position
#
proc setOrielFilter { id n } {
 set res -1
 catch {
   oriel_write_cmd $id FILT?
   after 500
   set res [oriel_read_result $id]
   set cpos [string range $res 4 4]
   set npos [expr $n - $cpos]
   while { $npos > 0 } {
      oriel_write_cmd $id NEXT
      incr npos -1
      after 2000
      set res [oriel_read_result $id]
   }
   while { $npos < 0 } {
      oriel_write_cmd $id PREV
      incr npos 1
      after 2000
      set res [oriel_read_result $id]
   }
   oriel_write_cmd $id FILT?
   after 500
   set res [oriel_read_result $id]
  }
  set cpos [string range $res 4 4]
  if { $cpos == $n } {
      return $n
  }
  return -1
}

## Documented proc \c checkAutoFilter .
#
#  Returns sequence for automatic filter cycling
#
proc checkAutoFilter { arm } {
global FWHEELS
   set aseq ""
   foreach i "1 2 3 4 5 6" {
      if { $FWHEELS([set arm]auto,$i) } {lappend aseq $i}
   }
   if { $aseq == "" } {return 0}
   return $aseq
}


## Documented proc \c reconnectFilters .
#
#  Reconnect to the filter wheels
#
proc reconnectFilters { } {
global FWHEELS FWSERIAL
  set f1 [oriel_connect 1]
  set sn1 $FWSERIAL([lindex $f1 1])
  if { $sn1 == $FWHEELS(red,serialnum) } {set FWHEELS(red,handle) 1}
  if { $sn1 == $FWHEELS(blue,serialnum) } {set FWHEELS(blue,handle) 1}

  set f2 [oriel_connect 2]
  set sn2 $FWSERIAL([lindex $f2 1])
  if { $sn2 == $FWHEELS(red,serialnum) } {set FWHEELS(red,handle) 2}
  if { $sn2 == $FWHEELS(blue,serialnum) } {set FWHEELS(blue,handle) 2}
  showstatus "Initializing filter wheel 1"
  resetFilterWheel 1
  showstatus "Initializing filter wheel 2"
  resetFilterWheel 2
}


## Documented proc \c filterEmccdGain .
#
#  Returns sequence for automatic filter cycling
#
proc filterEmccdGain { arm n em } {
global FWHEELS
   set FWHEELS($arm,$n,emgain) $em
   if { $arm == "red" } { .filters.remgain$n configure -text "EM=$em" }
   if { $arm == "blue" } { .filters.bemgain$n configure -text "EM=$em" }
}


# \endcode


foreach a "red blue" {
  foreach i "1 2 3 4 5 6" {
     set FWHEELS($a,$i,focus) 0
  }
}
set FWHEELS(focusoffset) 0
set SPECKLE_DIR $env(SPECKLE_DIR)

if { [info exists SCOPE(telescope)] == 0 } {
   proc mimicMode { a b c } {}
   proc showStatus { m } { puts stdout $m }
   proc debuglog { m } { puts stdout $m }
   proc setOrielFilter { id n } { return $n }
   set SCOPE(telescope) GEMINI
   set FWHEELS(sim) 1
   set FWHEELS(red,handle) 1
   set FWHEELS(blue,handle) 2
   load $env(SPECKLE_DIR)/lib/liboriel.so
   after 2000 wm deiconify .filters
}

set MAXFILTERS 6
destroy .filters
toplevel .filters -bg gray
wm title .filters "SPECKLE Filter Wheels control"
label .filters.lauto  -text "Auto" -bg gray -fg black
label .filters.lpos   -text "Red" -bg gray -fg black
label .filters.lname  -text "Red Filter Name" -bg gray -fg black
label .filters.lfocus -text "Focus offset" -bg gray -fg black
label .filters.lexpred -text "Exp" -bg gray -fg black
label .filters.lbauto  -text "Auto" -bg gray -fg black
label .filters.lbpos   -text "Blue" -bg gray -fg black
label .filters.lbname  -text "Blue Filter Name" -bg gray -fg black
label .filters.lbfocus -text "Focus offset" -bg gray -fg black
label .filters.lexpblue -text "Exp" -bg gray -fg black
place .filters.lauto -x 5 -y 20
place .filters.lbpos -x 80 -y 20
place .filters.lbname -x 180 -y 20
place .filters.lbfocus -x 330 -y 20
place .filters.lexpblue -x 510 -y 20
place .filters.lbauto -x 580 -y 20
place .filters.lpos -x 650 -y 20
place .filters.lname -x 730 -y 20
place .filters.lfocus -x 900 -y 20
place .filters.lexpred -x 1080 -y 20

wm geometry .filters 1160x[expr $MAXFILTERS*35 + 110]+10+170
set i 0
set iy 50
while { $i < $MAXFILTERS } {
   incr i 1
   checkbutton .filters.auto$i -bg gray -variable FWHEELS(redauto,$i) -highlightthickness 0
   button .filters.fred$i -text "$i" -relief raised -bg gray -fg black -command "selectfilter red $i" -width 8
   entry  .filters.namered$i -textvariable FWHEELS(red,$i) -bg LightBlue -fg black -width 20
   entry  .filters.focusred$i -textvariable FWHEELS(red,$i,focus) -bg LightBlue -fg black -width 8
   entry  .filters.expred$i -textvariable FWHEELS(red,$i,exp) -bg LightBlue -fg black -width 8
   place  .filters.auto$i -x 580 -y $iy
   place  .filters.fred$i -x 620 -y $iy
   menubutton .filters.remgain$i -width 7  -bg gray50  -menu .filters.remgain$i.m -relief raised -text "EM=0"
   menu .filters.remgain$i.m
   foreach ev "0 2 5 10 20 30 40 50 60 70 80 90 100 150 200 250 300 350 400 450 500 550 600 650 700 750 800 850 900 950 1000" {
     .filters.remgain$i.m add command -label $ev -command "filterEmccdGain red $i $ev"
   }
   place  .filters.remgain$i -x 990 -y $iy
   checkbutton .filters.bauto$i -bg gray -variable FWHEELS(blueauto,$i) -highlightthickness 0
   button .filters.fblue$i -text "$i" -relief raised -bg gray -fg black -command "selectfilter blue $i" -width 8
   entry  .filters.nameblue$i -textvariable FWHEELS(blue,$i) -bg LightBlue -fg black -width 20
   entry  .filters.focusblue$i -textvariable FWHEELS(blue,$i,focus) -bg LightBlue -fg black -width 8
   entry  .filters.expblue$i -textvariable FWHEELS(blue,$i,exp) -bg LightBlue -fg black -width 8
   place  .filters.fblue$i -x 50 -y $iy
   incr iy 3
   place  .filters.bauto$i -x 10 -y $iy
   place  .filters.namered$i -x 730 -y $iy
   place  .filters.focusred$i -x 910 -y $iy
   place  .filters.expred$i -x 1060 -y $iy
   place  .filters.nameblue$i -x 160 -y $iy
   place  .filters.focusblue$i -x 335 -y $iy
   place  .filters.expblue$i -x 485 -y $iy
   menubutton .filters.bemgain$i -width 7  -bg gray50  -menu .filters.bemgain$i.m -relief raised -text "EM=0"
   menu .filters.bemgain$i.m
   foreach ev "0 2 5 10 20 30 40 50 60 70 80 90 100 150 200 250 300 350 400 450 500 550 600 650 700 750 800 850 900 950 1000" {
     .filters.bemgain$i.m add command -label $ev -command "filterEmccdGain blue $i $ev"
   }
   place  .filters.bemgain$i -x 420 -y $iy
   incr iy 30
}

button .filters.load -text "Load configuration" -fg black -bg green -width 32 -command "loadFiltersConfig filtersConfiguration"
button .filters.save -text "Save configuration" -fg black -bg green -width 32 -command "saveFiltersConfig"
button .filters.exit -text "Close" -fg black -bg orange -width 32 -command "wm withdraw .filters"
place .filters.load -x 10 -y [expr $iy+30]
place .filters.save -x 350 -y [expr $iy+30]
place .filters.exit -x 700 -y [expr $iy+30]



loadFiltersConfig filtersConfiguration
#### in gui.tcl now load $env(SPECKLE_DIR)/lib/liboriel.so
set FWHEELS(red,init) 1
set FWHEELS(blue,init) 1

foreach p "1 2 3 4 5 6" {
  if { $FWHEELS(red,$p) == "Red-832" } {
     set FWHEELS(red,init) $p
  }
  if { $FWHEELS(blue,$p) == "Blue-562" } {
     set FWHEELS(blue,init) $p
  }
  set FWHEELS(red,$p,emgain) 0
  set FWHEELS(blue,$p,emgain) 0
  set FWHEELS(red,$p,exp) 0.06
  set FWHEELS(blue,$p,exp) 0.06
}


if { [info exists env(SPECKLE_SIM)] } {
   set simdev [split $env(SPECKLE_SIM) ,]
   if { [lsearch $simdev filter] > -1 } {
       set FWHEELS(sim) 1
       debuglog "Filter wheels in SIMULATION mode"
  }
}

if { $FWHEELS(sim) == 0 } {
  findWheels
  set f1 [oriel_connect 1]
  set sn1 $FWSERIAL([lindex $f1 1])
  if { $sn1 == $FWHEELS(red,serialnum) } {set FWHEELS(red,handle) 1}
  if { $sn1 == $FWHEELS(blue,serialnum) } {set FWHEELS(blue,handle) 1}

  set f2 [oriel_connect 2]
  set sn2 $FWSERIAL([lindex $f2 1])
  if { $sn2 == $FWHEELS(red,serialnum) } {set FWHEELS(red,handle) 2}
  if { $sn2 == $FWHEELS(blue,serialnum) } {set FWHEELS(blue,handle) 2}
  showstatus "Initializing filter wheel 1"
  resetFilterWheel 1

  showstatus "Initializing filter wheel 2"
  resetFilterWheel 2
  debuglog "Moving filter wheels to initial positions"
  selectfilter red $FWHEELS(red,init)
  selectfilter blue $FWHEELS(blue,init)
}  else {
  set FWHEELS(red,0) "simulate"
  set FWHEELS(blue,0) "simulate"
  set FWHEELS(red,position) 0
  set FWHEELS(blue,position) 0
}

wm withdraw .filters




