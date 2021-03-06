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

class argonautImportFile extends simplePlugin
{
  protected $daemon_events;
  protected $csv_fields = [
    '0' => 'TIMESTAMP',
    '1' => 'MAC',
    '2' => 'HEADER',
    '3' => 'OGROUP'
  ];

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Argonaut task import'),
      'plDescription' => _('Imports argonaut tasks from CSV file'),
      'plObjectType'  => ['argonautQueueImport' => [
        'name'        => _('Argonaut task import'),
        'aclCategory' => 'argonautQueue',
        'icon'        => 'geticon.php?context=applications&icon=argonaut&size=16',
      ]],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Import actions from CSV file'),
        'class' => ['fullwidth'],
        'attrs' => [
          new CompositeAttribute(
            _('Import a list of task into argonaut'),
            'import',
            [
              new FileAttribute(
                '', '',
                'import_file', FALSE
              ),
              new ButtonAttribute(
                '', '',
                'import_submit',
                _('Upload')
              )
            ],
            '', '%s%s', '',
            _('Import file')
          )
        ]
      ],
      'events' => [
        'name'      => _('Imported tasks'),
        'attrs'     => [new FakeAttribute('events')],
        'template'  => get_template_path('import_events.tpl', TRUE, dirname(__FILE__)),
      ],
    ];
  }

  public function __construct ($parent)
  {
    global $config;
    parent::__construct($config->current['BASE'], NULL, $parent);

    $this->attributesAccess['import']->setInLdap(FALSE);
    $this->attributesAccess['import']->setLinearRendering(TRUE);

    $this->events         = [];
    $this->daemon_events  = argonautEventTypes::get_event_types();
  }

  public function render (): string
  {
    $smarty = get_smarty();
    $smarty->assign('jobtypes', join(',', array_keys($this->daemon_events)));
    return parent::render().
    '<p class="plugbottom">'.
    ' <input type="submit" name="start_import" value="'._('Import').'" >'.
    ' <input type="submit" formnovalidate="formnovalidate" name="import_abort" value="'.msgPool::backButton().'">'.
    '</p>';
  }

  function handle_import_submit ()
  {
    $this->parse_csv($this->import);
  }

  public function readPost ()
  {
    /* Import started */
    if (isset($_POST['import_abort'])) {
      $this->parent->cancelEdit();
      return;
    } elseif (isset($_POST['start_import'])) {
      $events = $this->events;
      $failed = FALSE;
      foreach ($events as $event) {
        if (!empty($event['ERROR'])) {
          $failed = TRUE;
          break;
        }
      }
      if ($failed) {
        $error = new FusionDirectoryError(htmlescape(_('Fix the errors in your CSV file first')));
        $error->display();
      } else {
        $success  = 0;
        $fail     = 0;

        foreach ($events as $key => $event) {
          /* Create event */
          if (empty($event['TIMESTAMP'])) {
            $data = [];
          } else {
            $data = ['timestamp' => $event['TIMESTAMP']];
          }
          $this->parent->o_queue->append_call($event['HEADER'], [$event['MAC']], $data);
          if ($this->parent->o_queue->is_error()) {
            $error = new FusionDirectoryError(msgPool::siError($this->parent->o_queue->get_error()));
            $error->display();
            $fail++;
          } else {
            unset($events[$key]);
            $success++;
          }
        }
        msg_dialog::display(_('Import'), htmlescape(sprintf(_('Import complete: %s events successfully send, %s failed'), $success, $fail)), INFO_DIALOG);
        $this->events = $events;
      }
    }
    parent::readPost();
  }

  private function parse_csv ($str)
  {
    /* Some file checks */
    $lines = preg_split('/\n/', $str);
    if (empty($str) || !count($lines)) {
      $error = new FusionDirectoryError(msgPool::incorrectUpload(_('file is empty')));
      $error->display();
      return;
    }

    /* Reset current events */
    $events = [];

    /* Parse each line of the given file */
    foreach ($lines as $line) {
      /* Skip empty lines */
      if (empty($line)) {
        continue;
      }

      /* Load values from file */
      $fields = explode(';', $line);
      $event  = [];
      foreach ($this->csv_fields as $key => $val) {
        $event[$val] = '';
        if (isset($fields[$key])) {
          $event[$val] = $fields[$key];
        }
      }
      $event['ERROR'] = '';
      if (empty($event['MAC']) || !tests::is_mac($event['MAC'])) {
        $event['ERROR'] .= msgPool::invalid(_('MAC')).', ';
      }
      if (empty($event['HEADER']) || !isset($this->daemon_events[$event['HEADER']])) {
        $event['ERROR'] .= msgPool::invalid(_('Event')).', ';
      }
      $event['ERROR'] = trim($event['ERROR'], ', ');
      $events[] = $event;
    }
    $this->attributesAccess['events']->setPostValue($events);
  }
}
