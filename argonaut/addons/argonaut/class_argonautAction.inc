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

class MacsAttribute extends GenericDialogAttribute
{
  protected $dialogClass = 'SystemSelectDialog';

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = array(), $display_attr = 'cn', $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, 'macAddress', $display_attr, $acl);
  }
}

class argonautAction extends simplePlugin
{
  static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Argonaut task'),
      'plDescription' => _('Schedule an argonaut task'),
      'plObjectType'  => array('argonautTask' => array(
        'name'        => _('Argonaut task'),
        'aclCategory' => 'argonautQueue',
        'icon'        => 'geticon.php?context=applications&icon=argonaut&size=16',
      )),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    $events = argonautEventTypes::get_event_types();
    return array(
      'main' => array(
        'name'  => _('Schedule'),
        'attrs' => array(
          new SelectAttribute(
            _('Action'), _('The action you are about to schedule'),
            'action', TRUE,
            array_keys($events), '',
            array_map(
              function ($event)
              {
                return $event['name'];
              },
              $events
            )
          ),
          new BooleanAttribute(
            _('Scheduled'), _('Wether this job should be scheduled or ran right away'),
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

  public function __construct ($action, array $targets, $scheduled)
  {
    global $config;
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

    $this->action     = $action;
    $this->targets    = $targets;
    $this->scheduled  = $scheduled;
  }

  protected function loadAttributes ()
  {
    foreach ($this->attributesAccess as &$attribute) {
      $attribute->setInLdap(FALSE);
    }
    unset($attribute);
    parent::loadAttributes();
  }

  function execute (): string
  {
    return parent::execute().$this->getFooter();
  }

  /*! \brief  Returns the plugin footer (save cancel), displayed in the template.
    @return String  HTML footer part.
   */
  protected function getFooter ()
  {
    if (!$this->displayPlugin) {
      return '';
    }
    return  '<p class="plugbottom">'.
            ' <input type="submit" name="save_event_dialog" value="'.msgPool::saveButton().'"/>'.
            ' <input type="submit" formnovalidate="formnovalidate" name="abort_event_dialog" value="'.msgPool::cancelButton().'"/>'.
            '</p>';
  }

  /*! brief  Returns an array containing all attributes \
    That should be written.
    @return Array e.g. 'status' => 'bla blub'
   */
  public function computeData ()
  {
    timezone::get_default_timezone();
    $ret = array('args' => array());
    if ($this->scheduled) {
      $timestamp = strtotime($this->datetime);
      if ($timestamp === FALSE) {
        throw new FusionDirectoryException('Could not convert date to timestamp: "'.$this->datetime.'"');
      }
      $ret['timestamp'] = $timestamp;
      if ($this->periodical) {
        $ret['periodic'] = $this->period;
      }
    }

    return $ret;
  }
}
