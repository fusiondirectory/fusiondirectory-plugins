<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

class TimeHisAttribute extends CompositeAttribute
{
  function __construct($label, $description, $ldapName, $required, $acl = '')
  {
    $attributes = array(
      new IntAttribute (
        '',  _('Hours'),
        $ldapName.'_hours', TRUE,
        0, 23, 1
      ),
      new IntAttribute (
        ':', _('Minutes'),
        $ldapName.'_minutes', TRUE,
        0, 59, 0
      ),
      new IntAttribute (
        ':', _('Seconds'),
        $ldapName.'_seconds', TRUE,
        0, 59, 0
      )
    );
    parent::__construct($description, $ldapName, $attributes, '/^(\d\d)(\d\d)(\d\d)$/', '%2d%2d%2d', $acl, $label);
    $this->setLinearRendering(TRUE);
  }
}

class DateTimeAttribute extends CompositeAttribute
{
  function __construct($label, $description, $ldapName, $required, $acl = '')
  {
    $attributes = array(
      new DateAttribute(
        _('Date'), '',
        $ldapName.'_date', $required,
        'Ymd'
      ),
      new TimeHisAttribute(
        _('Time'), '',
        $ldapName.'_time', $required
      )
    );
    parent::__construct($description, $ldapName, $attributes, '/^(\d{8})(\d{6})$/', '%s%s', $acl, $label);
  }
}

class MacsAttribute extends GenericDialogAttribute
{
  protected $dialogClass = 'SystemSelectDialog';

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = array(), $display_attr = 'cn', $acl = '')
  {
    parent::__construct ($label, $description, $ldapName, $required, $defaultValue, 'macAddress', $display_attr, $acl);
  }
}

class argonautAction extends simplePlugin
{
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Argonaut task'),
      'plDescription' => _('Schedule an argonaut task'),
      'plObjectType'  => array('argonautTask' => array(
        'name'        => _('Argonaut task'),
        'aclCategory' => 'argonautQueue',
        'icon'        => 'geticon.php?context=applications&icon=argonaut&size=16',
      )),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Schedule'),
        'attrs' => array(
          new HiddenAttribute(
            'action'
          ),
          new HiddenAttribute(
            'scheduled', FALSE,
            TRUE
          ),
          new DateTimeAttribute(
            _('Schedule'), '',
            'datetime', FALSE
          ),
          new BooleanAttribute(
            _('Periodical job'), _('Wether this job should repeat in time'),
            'periodical', FALSE
          ),
          new CompositeAttribute(
            _('How often this task should be repeated'), 'period',
            array(
              new IntAttribute(
                '', '',
                'periodValue', FALSE,
                1, FALSE, 7
              ),
              new SelectAttribute(
                '', '',
                'periodType', TRUE,
                array('minutes',    'hours',    'days',     'weeks',    'months'), 'days',
                array(_('Minutes'), _('Hours'), _('Days'),  _('Weeks'), _('Months'))
              )
            ),
            '/^(\d+)_(minutes|hours|days|weeks|months)$/',
            '%s_%s',
            '',
            _('Period')
          )
        )
      ),
      'targets' => array(
        'name'  => _('Targets'),
        'attrs' => array(
          new MacsAttribute(
            '', _('Targets for this task'),
            'targets', TRUE
          )
        )
      )
    );
  }

  public function __construct($action)
  {
    parent::__construct($config->current['BASE']);

    $this->attributesAccess['scheduled']->setManagedAttributes(
      array(
        'erase' => array (
          FALSE => array (
            'datetime', 'period'
          )
        )
      )
    );
    $this->attributesAccess['periodical']->setManagedAttributes(
      array(
        'disable' => array (
          FALSE => array (
            'period'
          )
        )
      )
    );
    $this->attributesAccess['period']->setLinearRendering(TRUE);

    $this->datetime = date('YmdHis');

    $this->action = $action;
  }

  protected function loadAttributes()
  {
    foreach ($this->attributesAccess as &$attribute) {
      $attribute->setInLdap(FALSE);
    }
    unset($attribute);
    parent::loadAttributes();
  }

  function execute()
  {
    return parent::execute().$this->getFooter();
  }

  /*! \brief  Returns the plugin footer (save cancel), displayed in the template.
    @return String  HTML footer part.
   */
  protected function getFooter()
  {
    if (!$this->displayPlugin) {
      return '';
    }
    return  '<p class="plugbottom">'.
            ' <input type="submit" name="save_event_dialog" value="'.msgPool::saveButton().'"/>'.
            ' <input type="submit" name="abort_event_dialog" value="'.msgPool::cancelButton().'"/>'.
            '</p>';
  }

  /*! brief  Returns an array containing all attributes \
    That should be written.
    @return Array e.g. 'status' => 'bla blub'
   */
  public function computeData()
  {
    timezone::get_default_timezone();
    $ret = array('args' => array());
    if ($this->scheduled) {
      $ret['timestamp'] = strtotime($this->datetime);
      if ($this->periodical) {
        $ret['periodic'] = $this->period;
      }
    }

    return $ret;
  }

  /*! \brief  Returns event information, like menu strings, images ...
    @return   Array Event informations.
   */
  static public function get_event_info($action)
  {
    $events = self::get_event_types();
    if (isset($events[$action])) {
      $infos = $events[$action];

      $infos['menuimg'] = '<img src="'.$infos['img'].'" alt="'.$infos['name'].'" class="center"/>';
      $infos['listimg'] = '<img src="'.$infos['img'].'" title="'.$infos['name'].
                          '"alt="'.$infos['name'].'" class="center"/>';
      return $infos;
    } else {
      return FALSE;
    }
  }

  /*! \brief  Returns a complete list of all available events.
    @return   Array   Containing info for all available events.
   */
  static public function get_event_types()
  {
    return array(
      'System.halt' => array(
        'name'  => _('Switch off'),
        'img'   => 'geticon.php?context=actions&icon=system-shutdown&size=16'
      ),
      'Deployment.reboot' => array(
        'name'  => _('Reboot'),
        'img'   => 'geticon.php?context=actions&icon=system-reboot&size=16'
      ),
      'Deployment.wake' => array(
        'name'  => _('Wake up'),
        'img'   => 'geticon.php?context=status&icon=task-running&size=16'
      ),
      'Deployment.update' => array(
        'name'  => _('Software update'),
        'img'   => 'geticon.php?context=actions&icon=system-update&size=16'
      ),
      'Deployment.reinstall' => array(
        'name'  => _('(Re)Install'),
        'img'   => 'geticon.php?context=actions&icon=system-reinstall&size=16'
      ),
    );
  }
}
?>
