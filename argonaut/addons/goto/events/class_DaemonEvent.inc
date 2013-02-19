<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2006  Fabian Hickert
  Copyright (C) 2011  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

define("SCHEDULED_EVENT", 1);
define("TRIGGERED_EVENT", 2);

/*! \brief    This is the event base class
  \author   Fabian Hickert <hickert@gonicus.de>
  \version  1.00
  \date     26.02.2008

  This is the base class for all new daemon events.
  It implements most of the required functionality.
 */
class DaemonEvent
{
  /* Menu Strings */
  protected $s_Menu_Name  = "s_Menu_Name not set";  // Diplayed in the ActionsMenu->CreateNew
  protected $s_Event_Name = "s_Event_Name not set"; // Will be displayed in the Management list.
  protected $s_Menu_Image = "images/empty.png";     // Image displayed in Actions->New
  protected $s_List_Image = "";                     // Image displayed in event listing

  public $config;          // FusionDirectory configuration object
  protected $data;            // The event data, when edited

  protected $a_targets    = array();  // The list of assigned Targets (When newly created)
  protected $s_Action     = "";       // The deamon command name when edited. (e.g. trigger_action_halt)
  protected $timestamp    = 0;        // Event execution time;
  protected $id           = -1;       // The Table ID
  protected $status       = "unknown";// The current event status
  protected $is_new       = TRUE;     // Is TRUE if this is a new event

  protected $mode         = SCHEDULED_EVENT; // Default action is sheduled.

  /* Sub dialog hanlding */
  protected $target_divlist       = NULL;     // The divlist used by the target add dialog
  protected $target_add_list_used = FALSE;    // Indicates that the target add list was used.
  protected $time_select_used     = FALSE;    // Indicates that we have used the timestamp select boxes.
  protected $target_list_used     = FALSE;    // Target list was diaplayed?
  protected $_target_list         = array();  // Object Cache of those objects displayed in the target add dialog
  protected $workstation_list     = array();  // Used as cache in the target list.
  protected $server_list          = array();  // Used as cache in the target list.

  protected $periodType               = "days";
  protected $periodValue              = 7;
  protected $activate_periodical_job  = FALSE;

  protected $attributes = array("timestamp");

  function set_type($type)
  {
    $this->mode = $type;
  }


  function get_type()
  {
    return $this->mode;
  }

  /*! \brief  Class contructor.
    @param  Array   FusionDirectory configuration object.
    @param  Array   Event data, only given when edited.
   */
  public function __construct($config, $action, $data = array())
  {
    $this->data     = $data;
    $this->config   = $config;
    $this->s_Action = $action;
    timezone::get_default_timezone();
    $this->timestamp = 0;

    /* Load values from given data */
    if (count($data)) {
      $this->is_new = FALSE;

      $this->id = $data['ID'];
      if (isset($data['TIMESTAMP'])) {
        $this->timestamp = $this->_event_to_timestamp($data['TIMESTAMP']);
      }
    }

    if (isset($data['PERIODIC']) && !preg_match("/none/i", $data['PERIODIC'])) {
      $tmp = explode("_", $data['PERIODIC']);
      if (count($tmp) == 2) {
        $this->activate_periodical_job = TRUE;

        $this->periodValue  = $tmp[0];
        $this->periodType   = $tmp[1];
      }
    }
  }


  /*! \brief  Create the HTML output for the plugin.
    @return String  The generated HTML output.
   */
  public function execute()
  {
    $this->time_select_used = FALSE;
    $this->target_list_used = FALSE;

    $display = $this->get_header();

    $tmp = $this->data;

    /* Check if target add dialog is open */
    if ($this->is_target_list_open() && $this->is_new) {
      return $this->get_target_add_list();
    }

    $smarty = get_smarty();
    $smarty->assign("data",         $this->data);
    $smarty->assign("target_list",  $this->get_target_list());
    $smarty->assign("is_new",       $this->is_new);
    $smarty->assign("timestamp",    $this->get_time_select());
    $display .= $smarty->fetch(get_template_path('DaemonEvent.tpl', TRUE, dirname(__FILE__)));
    $display .= $this->get_footer();
    return $display;
  }

  /*! \brief  Returns the plugin header, displayed in the template.
    @return String  HTML header part.
   */
  public function get_header()
  {
    if ($this->target_add_list_used) {
      return "";
    }
    $str = "<h2>".sprintf(_("Create '%s' job"), $this->s_Event_Name)."</h2>";
    return $str;
  }


  /*! \brief  Returns the plugin footer (save cancel), displayed in the template.
    @return String  HTML footer part.
   */
  public function get_footer()
  {
    if ($this->target_add_list_used) {
      return "";
    }
    $str = "<p class='seperator'></p>";
    $str .= "<div style='text-align:right;width:99%;padding:5px;'>
      <input type='submit' name='save_event_dialog' value='".msgPool::saveButton()."'>&nbsp;
    <input type='submit' name='abort_event_dialog' value='".msgPool::cancelButton()."'>
      </div>";
    return $str;
  }


  /*! \brief  Returns HTML representation of a timestamp using <select> boxes.
    @return Returns HTML content.
   */
  public function get_time_select()
  {
    timezone::get_default_timezone();
    $this->time_select_used = TRUE;

    $smarty = get_smarty();

    $year   = date("Y", $this->timestamp);
    $month  = date("m", $this->timestamp);
    $day    = date("d", $this->timestamp);

    $hour   = date("H", $this->timestamp);
    $minute = date("i", $this->timestamp);
    $second = date("s", $this->timestamp);

    $years = array();
    for ($i = date("Y", time()); $i <= 2037; $i++) {
      $years[$i] = $i;
    }
    $months = array();
    for ($i = 1; $i <= 12; $i++) {
      $e = str_pad($i, 2, "0", STR_PAD_LEFT);
      $months[$e] = $e;
    }
    $days = array();
    for ($i = 1; $i <= cal_days_in_month(CAL_GREGORIAN, $month, $year); $i++) {
      $e = str_pad($i, 2, "0", STR_PAD_LEFT);
      $days[$e] = $e;
    }
    $hours = array();
    for ($i = 0; $i < 24; $i++) {
      $e = str_pad($i, 2, "0", STR_PAD_LEFT);
      $hours[$e] = $e;
    }
    $minutes = array();
    for ($i = 0; $i < 60; $i++) {
      $e = str_pad($i, 2, "0", STR_PAD_LEFT);
      $minutes[$e] = $e;
    }
    $seconds = array();
    for ($i = 0; $i < 60; $i++) {
      $e = str_pad($i, 2, "0", STR_PAD_LEFT);
      $seconds[$e] = $e;
    }

    $periodTypes = array(
        "minutes" => _("Minutes"),
        "hours"   => _("Hours"),
        "days"    => _("Days"),
        "weeks"   => _("Weeks"),
        "months"  => _("Months"));


    $smarty->assign("periodTypes", $periodTypes);
    $smarty->assign("periodType", $this->periodType);
    $smarty->assign("periodValue", $this->periodValue);
    $smarty->assign("activate_periodical_job", $this->activate_periodical_job);

    $smarty->assign("years", $years);
    $smarty->assign("months", $months);
    $smarty->assign("days", $days);
    $smarty->assign("hours", $hours);
    $smarty->assign("minutes", $minutes);
    $smarty->assign("seconds", $seconds);
    $smarty->assign("time_year", $year);
    $smarty->assign("time_month", $month);
    $smarty->assign("time_day", $day);
    $smarty->assign("time_hour", $hour);
    $smarty->assign("time_minute", $minute);
    $smarty->assign("time_second", $second);
    return $smarty->fetch(get_template_path('timestamp_select.tpl', TRUE, dirname(__FILE__)));
  }


  /*! \brief  HTML representation of all currently assigned targets using (divSelectBox).
    @return String Returns a listbox with all used targets.
   */
  public function get_target_list()
  {
    $this->target_list_used = TRUE;
    $divlist = new divSelectBox("EventTargets");
    foreach ($this->a_targets as $key => $target) {
      $divlist->AddEntry(array(
            array("string"  => $target),
            array("string"  => "<input type='image' src='images/lists/trash.png' title='"._("Remove")."' name='del_target_".$key."'>",
              "attach"  => "style='width:20px; border-right:0px;'")
            ));
    }
    $list_footer = "<input type='submit' name='open_target_list' value='"._("Add")."'>";
    return $divlist->DrawList().$list_footer;
  }


  /*! \brief  Returns HTML content, displaying a dialog which allows to add new targets.
    @return String HTML content. (EventAddSystemDialog)
   */
  public function get_target_add_list()
  {
    $this->target_add_list_used = TRUE;

    if ($this->target_divlist == NULL) {
      $this->target_divlist = new EventAddSystemDialog($this->config, $this);
    }

    $smarty = get_smarty();
    $smarty->assign("divlist", $this->target_divlist->execute());
    return $smarty->fetch(get_template_path('target_list.tpl', TRUE, dirname(__FILE__)));
  }


  /*! \brief  Handles all posted HTML data, including target add,remove...
   */
  public function save_object()
  {
    timezone::get_default_timezone();
    if (isset($_POST['open_target_list'])) {
      $this->target_add_list_used = TRUE;
    }
    if ($this->target_divlist != NULL) {
      $this->target_divlist->save_object();
    }
    if ($this->target_add_list_used) {
      if (isset($_POST['abort_target_dialog'])) {
        $this->target_add_list_used = FALSE;
        $this->target_divlist       = NULL;
      }
      if (isset($_POST['save_target_dialog'])) {
        $this->target_add_list_used = FALSE;
        $this->add_targets($this->target_divlist->get_selected_targets());
        $this->target_divlist = NULL;
      }
    }

    if ($this->time_select_used) {
      $time_stamp_values_found = TRUE;
      foreach (array("time_year","time_month","time_day","time_hour","time_minute","time_second") as $attr) {
        $time_stamp_values_found &= isset($_POST[$attr]);
      }
      if ($time_stamp_values_found) {
        $this->timestamp = mktime(
            $_POST['time_hour'],
            $_POST['time_minute'],
            $_POST['time_second'],
            $_POST['time_month'],
            $_POST['time_day'],
            $_POST['time_year']);

        if (isset($_POST['periodValue'])) {
          $this->periodValue = get_post('periodValue');
        }
        if (isset($_POST['periodType'])) {
          $this->periodType = get_post('periodType');
        }

        if (isset($_POST['activate_periodical_job'])) {
          $this->activate_periodical_job = TRUE;
        } else {
          $this->activate_periodical_job = FALSE;
        }
      }
    }

    if ($this->target_list_used) {
      foreach ($_POST as $name => $value) {
        if (preg_match("/^del_target_/", $name)) {
          $id = preg_replace("/^del_target_([0-9]*)_.*/", "\\1", $name);
          if (isset($this->a_targets[$id])) {
            unset($this->a_targets[$id]);
          }
          break;
        }
      }
    }
  }


  /*! \brief  Converts a daemon timestamp into an unix timestamp. \
    e.g.  20080101125959 -> 1199188799
    @param  A daemon timestamp  YYYYddmmHHiiss
    @return Integer  A unix timestamp.
   */
  static public function _event_to_timestamp($str)
  {
    timezone::get_default_timezone();
    return strtotime($str);
  }


  /*! \brief  Converts a unix timestamp in to a goto-si timestamp. \
    e.g.  1199188799 -> 20080101125959
    @param  A unix timestamp (e.g. 1199188799)
    @return Integer  A daemon timestamp (e.g. 20080101125959).
   */
  static public function _timestamp_to_event($stamp)
  {
    timezone::get_default_timezone();
    return date("YmdHis", $stamp);
  }


  /*! \brief  Returns event information, like menu strings, images ...
    @return   Array Event informations.
   */
  static public function get_event_info($action)
  {
    $events = self::get_event_types();
    if (isset($events[$action])) {
      $infos = $events[$action];

      $infos['menuimg'] = "<img src='".$infos['img']."' alt='".$infos['name']."' border='0' class='center'/>";
      $infos['listimg'] = "<img src='".$infos['img']."' title='".$infos['name'].
                          "'alt='".$infos['name']."' border='0' class='center'/>";
      return $infos;
    } else {
      return FALSE;
    }
  }


  /*! \brief  Check if we have opened the target add dialog.
    @return   Boolean TRUE if we have opened the target add dialog else FALSE.
   */
  protected function is_target_list_open()
  {
    return $this->target_add_list_used;
  }

  /*! \brief  Returns a complete list of all available events.
    @return   Array   Containing info for all available events.
   */
  static function get_event_types()
  {
    return array(
      'System.halt' => array(
        'name'  => _('Switch off'),
        'img'   => 'images/lists/off.png'
      ),
      'Deployment.reboot' => array(
        'name'  => _('Reboot'),
        'img'   => 'images/lists/reload.png'
      ),
      'Deployment.wake' => array(
        'name'  => _('Wake up'),
        'img'   => 'images/lists/on.png'
      ),
      'Deployment.update' => array(
        'name'  => _('Software update'),
        'img'   => 'plugins/systems/images/update.png'
      ),
      'Deployment.reinstall' => array(
        'name'  => _('Reinstall'),
        'img'   => 'plugins/systems/images/reinstall.png'
      ),
    );
  }


  /*! \brief  Returns TRUE if this event is new. (Not edited)
    @return Boolean TRUE if new, else FALSE.
   */
  public function is_new()
  {
    return $this->is_new;
  }

  /*! \brief  Returns the event tag to schedule a new action
    @param    Returns the event e.g. 'wake'
   */
  public function get_action()
  {
    return $this->s_Action;
  }

  /*! \brief  Returns the event tag to schedule a new action
    @param    Returns the event e.g. 'wake'
   */
  public function get_schedule_action()
  {
    trigger_error('deprecated');
    return $this->s_Action;
  }

  /*! \brief  Returns the event tag to schedule a new action
    @param    Returns the event e.g. 'trigger_action_wake'
   */
  public function get_trigger_action()
  {
    trigger_error('deprecated');
    return $this->s_Action;
  }


  /*! brief  Returns an array containig all attributes \
    That should be written.
    @return Array e.g. 'status' => 'bla blub'
   */
  public function save()
  {
    $ret = array();
    foreach ($this->attributes as $attr) {
      $ret[$attr] = $this->$attr;
    }
    if ($this->mode == SCHEDULED_EVENT) {
      $ret['timestamp'] = $this->_timestamp_to_event($this->timestamp);
    } elseif (isset($ret['timestamp'])) {
      unset($ret['timestamp']);
    }

    if ($this->activate_periodical_job) {
      $ret['periodic'] = $this->periodValue."_".$this->periodType;
    } elseif (isset($ret['periodic'])) {
      unset($ret['periodic']);
    }

    return $ret;
  }


  /*! \brief  Returns the event targets
    @return Array  All selected targets.
   */
  public function get_targets()
  {
    return $this->a_targets;
  }


  /*! \brief  Returns the event timestamp in goto-si format.
    @return Returns the event timestamp (20081231120000)
   */
  public function get_timestamp($si_type = TRUE)
  {
    if ($si_type) {
      return $this->_timestamp_to_event($this->timestamp);
    } else {
      return $this->timestamp;
    }
  }


  /*! \brief  Returns the event ID
    @return Returns the event ID
   */
  public function get_id()
  {
    if ($this->is_new) {
      return -1;
    } else {
      return $this->data['ID'];
    }
  }


  /*! \brief Add a target MAC address
      @param Array A List of all target that should be added.
   */
  public function set_timestamp($stamp)
  {
    $this->timestamp = $stamp;
  }

  /*! \brief set timestamp to actual time
      @param Array A List of all target that should be added.
   */
  public function set_timestamp_now()
  {
    timezone::get_default_timezone();
    $this->timestamp = time();
  }


  /*! \brief Add a target MAC address
      @param Array A List of all target that should be added.
   */
  public function add_targets($targets)
  {
    foreach ($targets as $target) {
      $this->a_targets[] = $target;
    }
  }

  public function check()
  {
    return array();
  }


  /*! \brief Update a class variable from outside
   */
  public function set_value($name, $value)
  {
    $name = strtolower($name);
    if (isset($this->$name) && in_array($name, $this->attributes)) {
      $this->$name = $value;
    }
  }
}
?>
