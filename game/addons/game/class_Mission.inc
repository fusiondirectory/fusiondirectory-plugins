<?php

class Mission
{
  private $title;
  private $objective;
  private $objectives;
  private $init_f;

  function __construct($title, $objectives, $init)
  {
    $this->title      = $title;
    $this->objective  = 0;
    $this->objectives = array();
    foreach ($objectives as $func => $label) {
      $this->objectives[] = array('func' => $func, 'label' => $label, 'done' => FALSE);
    }
    $this->init_f     = $init;
  }

  /* Starts the mission */
  public function init()
  {
    $func = $this->init_f;
    if (!empty($this->init_f)) {
      call_user_func($this->init_f);
    }
  }

  /* Returns true if mission complete */
  public function validate()
  {
    foreach ($this->objectives as &$obj) {
      if (call_user_func($obj['func'])) {
        $obj['done'] = TRUE;
      } else {
        return FALSE;
      }
    }
    return TRUE;
  }

  public function compute_infos()
  {
    return array(
      'title' => $this->title,
      'objectives' => $this->objectives
    );
  }
}

?>
