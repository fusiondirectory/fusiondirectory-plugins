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
  protected $csv_fields = array(
    '0' => 'TIMESTAMP',
    '1' => 'MAC',
    '2' => 'HEADER',
    '3' => 'OGROUP'
  );

  static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Argonaut task import'),
      'plDescription' => _('Imports argonaut tasks from CSV file'),
      'plObjectType'  => array('argonautQueueImport' => array(
        'name'        => _('Argonaut task import'),
        'aclCategory' => 'argonautQueue',
        'icon'        => 'geticon.php?context=applications&icon=argonaut&size=16',
      )),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    return array(
      'main' => array(
        'name'  => _('Import actions from CSV file'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new CompositeAttribute(
            _('Import a list of task into argonaut'),
            'import',
            array(
              new FileAttribute(
                '', '',
                'import_file', FALSE
              ),
              new ButtonAttribute(
                '', '',
                'import_submit',
                _('Upload')
              )
            ),
            '', '%s%s', '',
            _('Import file')
          )
        )
      ),
      'events' => array(
        'name'      => _('Imported tasks'),
        'attrs'     => array(new FakeAttribute('events')),
        'template'  => get_template_path('import_events.tpl', TRUE, dirname(__FILE__)),
      ),
    );
  }

  public function __construct ($parent)
  {
    global $config;
    parent::__construct($config->current['BASE'], NULL, $parent);

    $this->attributesAccess['import']->setInLdap(FALSE);
    $this->attributesAccess['import']->setLinearRendering(TRUE);

    $this->events         = array();
    $this->daemon_events  = argonautEventTypes::get_event_types();
  }

  function execute (): string
  {
    $smarty = get_smarty();
    $smarty->assign('jobtypes', join(',', array_keys($this->daemon_events)));
    return parent::execute().
    '<p class="plugbottom">'.
    ' <input type="submit" name="start_import" value="'._('Import').'" >'.
    ' <input type="submit" formnovalidate="formnovalidate" name="import_abort" value="'.msgPool::backButton().'">'.
    '</p>';
  }

  function handle_import_submit ()
  {
    $this->parse_csv($this->import);
  }

  function save_object ()
  {
    /* Import started */
    if (isset($_POST['start_import'])) {
      $events = $this->events;
      $error  = FALSE;
      foreach ($events as $event) {
        if (!empty($event['ERROR'])) {
          $error = TRUE;
          break;
        }
      }
      if ($error) {
        msg_dialog::display(
          _('Import'),
          _('Fix the errors in your CSV file first'),
          ERROR_DIALOG
        );
      } else {
        $success  = 0;
        $fail     = 0;

        foreach ($events as $key => $event) {
          /* Create event */
          if (empty($event['TIMESTAMP'])) {
            $data = array();
          } else {
            $data = array('timestamp' => $event['TIMESTAMP']);
          }
          $this->parent->o_queue->append_call($event['HEADER'], array($event['MAC']), $data);
          if ($this->parent->o_queue->is_error()) {
            msg_dialog::display(_('Infrastructure service'), msgPool::siError($this->parent->o_queue->get_error()), ERROR_DIALOG);
            $fail++;
          } else {
            unset($events[$key]);
            $success++;
          }
        }
        msg_dialog::display(_('Import'), sprintf(_('Import complete: %s events successfully send, %s failed'), $success, $fail), INFO_DIALOG);
        $this->events = $events;
      }
    }
    parent::save_object();
  }

  private function parse_csv ($str)
  {
    /* Some file checks */
    $lines = preg_split('/\n/', $str);
    if (empty($str) || !count($lines)) {
      msg_dialog::display(_('Import'), msgPool::incorrectUpload(_('file is empty')), ERROR_DIALOG);
      return;
    }

    /* Reset current events */
    $events = array();

    /* Parse each line of the given file */
    foreach ($lines as $line) {
      /* Skip empty lines */
      if (empty($line)) {
        continue;
      }

      /* Load values from file */
      $fields = explode(';', $line);
      $event  = array();
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
