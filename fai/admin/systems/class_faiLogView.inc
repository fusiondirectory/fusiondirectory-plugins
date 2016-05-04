<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class AvailableLogsAttribute extends Attribute
{
  var $logs     = array();

  function __construct ($description, $ldapName, $acl = "")
  {
    $value = array(
      'file'      => '',
      'time'      => '',
      'sort_by'   => 'time',
      'sort_up'  => TRUE // TRUE means up
    );
    parent::__construct ('', $description, $ldapName, FALSE, $value, $acl);
    $this->setInLdap(FALSE);
  }

  private function js_link($label, $vars)
  {
    $id   = $this->getHtmlId();
    $js = '';
    foreach ($vars as $var => $value) {
      $js .= 'document.mainform.'.$id.'_'.$var.'.value="'.$value.'";';
    }
    $js .= 'document.mainform.submit();';
    return '<a href="javascript:'.htmlentities($js, ENT_COMPAT, 'UTF-8').'">'.$label."</a>";
  }

  function renderFormInput ()
  {
    $id   = $this->getHtmlId();
    $div  = new divSelectBox($id);

    /* Create sort direction images */
    if ($this->value['sort_up']) {
      $img = '<img src="geticon.php?context=actions&size=16&icon=view-sort-descending" alt="'._('Sort down').'"/>';
    } else {
      $img = '<img src="geticon.php?context=actions&size=16&icon=view-sort-ascending" alt="'._('Sort up').'"/>';
    }
    if ($this->value['sort_by'] == 'file') {
      $img1 = $img;
      $img2 = '';
    } else {
      $img1 = '';
      $img2 = $img;
    }

    /* Create list header */
    $div->SetHeaders(array(
      $this->js_link(_('File')." $img1", array('sort_by' => 'file')),
      $this->js_link(_('Date')." $img2", array('sort_by' => 'time'))
    ));

    /* Create sortable array */
    $to_add   = array();
    $sort_by  = $this->value['sort_by'];
    foreach ($this->logs as $mac => $times) {
      foreach ($times as $time => $data) {
        $rtime = $data['REAL_DATE'];
        foreach ($data['FILES'] as $file) {
          $highlight = '';
          if (($file == $this->value['file']) && ($time == $this->value['time']) && ($mac == $this->plugin->mac)) {
            $highlight = 'background-color:#CCCCCC';
          }

          $use_link = $this->js_link('%str%', array('time' => $time, 'file' => $file, 'mac' => $mac));
          $to_add[$$sort_by.$file.$time] = array(
            array('html' => preg_replace('/%str%/', $file, $use_link),
                  'attach' => ''),
            array('html' => preg_replace('/%str%/', date('d.m.Y H:i:s', $rtime), $use_link),
                  'attach' => ''),
          );
        }
      }
    }

    /* Sort entries */
    uksort($to_add, 'strnatcasecmp');
    if ($this->value['sort_up']) {
      $to_add = array_reverse($to_add);
    }

    $div->SetHeight(200);

    /* Append entries to list */
    foreach ($to_add as $entry) {
      $div->AddEntry($entry);
    }

    $smarty = get_smarty();
    $smarty->assign("div_$id", $div->DrawList());
    $display = '';
    foreach (array('sort_by','time','file','mac') as $var) {
      $display .= '<input type="hidden" value="" name="'.$id.'_'.$var.'" id="'.$id.'_'.$var.'"/>'."\n";
    }
    return $this->renderAcl($display).'{$div_'.$id.'}'."\n";
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      $id   = $this->getHtmlId();
      $this->postValue = $this->value;
      foreach (array('time','file') as $attr) {
        if (isset($_POST[$id.'_'.$attr])) {
          $this->postValue[$attr] = $_POST[$id.'_'.$attr];
        }
      }
      if (isset($_POST[$id.'_sort_by']) && in_array($_POST[$id.'_sort_by'], array('file','time'))) {
        if ($_POST[$id.'_sort_by'] == $this->postValue['sort_by']) {
          $this->postValue['sort_up'] = !$this->postValue['sort_up'];
        }
        $this->postValue['sort_by'] = $_POST[$id.'_sort_by'];
      }
    }
  }
}

class faiLogView extends simplePlugin
{
  var $mac;
  var $o_queue;

  var $displayHeader  = FALSE;

  static function getAttributesInfo ()
  {
    return array(
      'available' => array(
        'name'  => _('Available logs'),
        'attrs' => array(
          new AvailableLogsAttribute(
            _('Available logs'), 'available_logs'
          ),
        )
      ),
      'selected' => array(
        'name'  => _('Selected log'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new DisplayAttribute(
            '', _('Content of the selected log'),
            'display_log'
          ),
        )
      )
    );
  }

  /* Return plugin informations */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('FAI Logs'),
      'plDescription' => _('FAI Logs Viewer'),
      'plPriority'    => 30,
      'plObjectType'  => array('workstation', 'server', 'ogroup-dynamic'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  function __construct($unused, $dn, $object)
  {
    /* Try to fetch logs for the given event (mac) */
    $this->o_queue = new supportDaemon();

    /* Load ldap object if given and use this macAddress. */
    if (is_object($object) && $dn != "" && $dn != "new") {
      parent::__construct($unused, $dn, $object);
    } else {
      parent::__construct($unused, 'new', $object);
    }

    /* Get correct macAddress. Check if an event is given or a ldap object. */
    if (isset($this->attrs['macAddress'][0])) {
      $this->mac = $this->attrs['macAddress'][0];
    }

    $this->is_account = FALSE;
    if ($this->mac != '') {
      /* Query for log files */
      $res = $this->o_queue->get_log_info_for_mac($this->mac);
      if ($this->o_queue->is_error()) {
          msg_dialog::display(_("Error"), $this->o_queue->get_error(), ERROR_DIALOG);
      }

      /* Check if there is at least one log file */
      if (isset($res[$this->mac]) && is_array($res[$this->mac])) {
        $this->attributesAccess['available_logs']->logs = $res;
        $this->is_account = TRUE;
      }
    }
  }


  function execute()
  {
    if ($this->available_logs['file'] == '') {
      $this->attributesInfo['selected']['name'] = _('Selected file');
      $this->display_log = '';
    } else {
      $label = $this->available_logs['file'];
      if (isset($this->attributesAccess['available_logs']->logs[$this->mac][$this->available_logs['time']]['REAL_DATE'])) {
        $date   = date('d.m.Y H:i:s', $this->attributesAccess['available_logs']->logs[$this->mac][$this->available_logs['time']]['REAL_DATE']);
        $label  = $label.', '.$date;
      }
      $this->attributesInfo['selected']['name'] = sprintf(_('Selected file: %s'), $label);
      $this->display_log = $this->get_log($this->mac, $this->available_logs['time'], $this->available_logs['file']);
    }

    return parent::execute();
  }


  private function get_log($mac, $date, $file)
  {
    $res = $this->o_queue->get_log_file($mac, $date, $file);
    if ($this->o_queue->is_error()) {
      msg_dialog::display(_("Error"), $this->o_queue->get_error(), ERROR_DIALOG);
    }
    $res = nl2br(htmlentities($res));
    return $res;
  }
}
?>
