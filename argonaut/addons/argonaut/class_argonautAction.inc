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

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $display_attr = 'cn', $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, 'macAddress', $display_attr, $acl);
  }
}

class argonautAction extends ManagementDialog
{
  protected $post_cancel = 'abort_event_dialog';
  protected $post_finish = 'save_event_dialog';

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Argonaut task'),
      'plDescription' => _('Schedule an argonaut task'),
      'plObjectType'  => ['argonautTask' => [
        'name'        => _('Argonaut task'),
        'aclCategory' => 'argonautQueue',
        'icon'        => 'geticon.php?context=applications&icon=argonaut&size=16',
      ]],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    $events = argonautEventTypes::get_event_types();
    return [
      'main' => [
        'name'  => _('Schedule'),
        'attrs' => [
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
            [
              new IntAttribute(
                '', '',
                'periodValue', FALSE,
                1, FALSE, 7
              ),
              new SelectAttribute(
                '', '',
                'periodType', TRUE,
                ['minutes',    'hours',    'days',     'weeks',    'months'], 'days',
                [_('Minutes'), _('Hours'), _('Days'),  _('Weeks'), _('Months')]
              )
            ],
            '/^(\d+)_(minutes|hours|days|weeks|months)$/',
            '%s_%s',
            '',
            _('Period')
          )
        ]
      ],
      'targets' => [
        'name'  => _('Targets'),
        'attrs' => [
          new MacsAttribute(
            '', _('Targets for this task'),
            'targets', TRUE
          )
        ]
      ]
    ];
  }

  public function __construct (management $parent, string $action, array $targets, bool $scheduled)
  {
    global $config;
    parent::__construct($config->current['BASE'], NULL, $parent);

    $this->attributesAccess['scheduled']->setManagedAttributes(
      [
        'erase' => [
          FALSE => [
            'datetime', 'period'
          ]
        ]
      ]
    );
    $this->attributesAccess['periodical']->setManagedAttributes(
      [
        'disable' => [
          FALSE => [
            'period'
          ]
        ]
      ]
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

  protected function handleFinish ()
  {
    $msgs = $this->check();
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
      return;
    }
    if ($this->parent->siActive) {
      $o_queue = new supportDaemon();
      $o_queue->append($this);
      if ($o_queue->is_error()) {
        $error = new FusionDirectoryError(msgPool::siError($o_queue->get_error()));
        $error->display();
      }
      $this->parent->closeDialogs();
    }
  }

  /*! brief  Returns an array containing all attributes \
    That should be written.
    @return Array e.g. 'status' => 'bla blub'
   */
  public function computeData ()
  {
    timezone::get_default_timezone();
    $ret = ['args' => []];
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
