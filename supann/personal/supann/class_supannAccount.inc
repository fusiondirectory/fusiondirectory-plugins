<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2010-2012 Antoine Gallavardin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

class supannAccount extends plugin
{
  var $plHeadline     = "supann";
  var $plDescription  = "Plugin supann";
  var $plIcon  = "plugins/supann/images/plugin.png";

  var $view_logged            = FALSE;
  var $is_account             = FALSE;
  var $initially_was_account  = FALSE;

  var $type_profil                        = "";

  var $eduPersonAffiliation               = "";
  var $eduPersonPrimaryAffiliation        = "";
  var $eduPersonNickName                  = "";
  var $supannEntiteAffectation            = "";
  var $supannEntiteAffectationPrincipale  = "";
  var $eduPersonPrincipalName             = "";
  var $supannAliasLogin                   = "";
  var $supannAutreTelephone               = "";
  var $supannCivilite                     = "";
  var $supannEtablissement                = "";
  var $supannAutreMail                    = "";
  var $supannListeRouge                   = "";
  var $supannMailPerso                    = "";
  var $supannTypeEntiteAffectation        = "";
  var $supannEmpCorps                     = "";
  var $supannEmpId                        = "";
  var $supannRoleGenerique                = "";
  var $supannRoleEntite                   = "";
  var $supannActivite                     = "";
/*
  var $supannParrainDN                    = "";
  var $supannRefId                        = "";
*/
  var $supannEtuId                        = "";

  var $list_supannEtuSecteurDisciplinaire_SEI                = array();

  var $list_supannEtuCursusAnnee = array(
                                          "L" => "Licence",
                                          "M" => "Master",
                                          "D" => "Doctorat",
                                          "X" => "Autre catégorie de diplome",
                                          "B" => "année post-bac"
                                        );

  var $years_supannEtuCursusAnnee = array(
                                          "L" => 3,
                                          "M" => 2,
                                          "D" => 10,
                                          "X" => 10,
                                          "B" => 20
                                        );

  var $attributes =
    array("eduPersonAffiliation", "eduPersonPrimaryAffiliation",
          "eduPersonNickName", "eduPersonPrincipalName",
          "supannEntiteAffectation", "supannEntiteAffectationPrincipale",
          "supannAliasLogin", "supannAutreMail",
          "supannAutreTelephone", "supannCivilite",
          "supannEtablissement", "supannListeRouge",
          "supannMailPerso",
/*
          "supannParrainDN",
          "supannRefId",
*/

          /* Student attributes */
          "supannTypeEntiteAffectation", "supannCodeINE",
          "supannEtuId",

          "supannEtuInscription",
          /* Inscription sub attributes */
          "supannEtuAnneeInscription", "supannEtuRegimeInscription",
          "supannEtuSecteurDisciplinaire", "supannEtuTypeDiplome",
          "supannEtuCursusAnnee",
          /* optional ones */
          "supannEtuDiplome", "supannEtuEtape",
          "supannEtuElementPedagogique",

          /* Personal attributes */
          "supannEmpCorps", "supannEmpId",
          "supannRoleGenerique", "supannRoleEntite",
          "supannActivite",
        );

  /* supannEtuInscription */
  var $SEI_attributes = array(
    "supannEtablissement" => array(
                                    /* Does this attribute looks like {ref}value */
                                    'ref'=>TRUE,
                                    /* Does this attribute needs a list of available values */
                                    'list'=>TRUE,
                                    /* Name of this attribute in the SEI entry */
                                    'shortname'=>"etab"
                                  ),
    "supannEtuAnneeInscription"     => array('ref'=>FALSE,'list'=>FALSE,'shortname'=>"anneeinsc"),
    "supannEtuRegimeInscription"    => array('ref'=>TRUE, 'list'=>TRUE, 'shortname'=>"regimeinsc"),
    "supannEtuSecteurDisciplinaire" => array('ref'=>TRUE, 'list'=>TRUE, 'shortname'=>"sectdisc"),
    "supannEtuTypeDiplome"          => array('ref'=>TRUE, 'list'=>TRUE, 'shortname'=>"typedip"),
    "supannEtuCursusAnnee"          => array('ref'=>FALSE,'list'=>FALSE,'shortname'=>"cursusann"),
    "supannEntiteAffectation"       => array('ref'=>FALSE,'list'=>TRUE, 'shortname'=>"affect"),
    "supannEtuDiplome"              => array('ref'=>TRUE, 'list'=>TRUE, 'shortname'=>"diplome"),
    "supannEtuEtape"                => array('ref'=>TRUE, 'list'=>TRUE, 'shortname'=>"etape"),
    "supannEtuElementPedagogique"   => array('ref'=>TRUE, 'list'=>TRUE, 'shortname'=>"eltpedago"),
  );

  /* ref of chosen value for SEI attribute */
  var $ref_supannEtablissement_SEI;
  //var $ref_supannEtuAnneeInscription_SEI;
  var $ref_supannEtuRegimeInscription_SEI;
  var $ref_supannEtuSecteurDisciplinaire_SEI;
  var $ref_supannEtuTypeDiplome_SEI;
  //var $ref_supannEntiteAffectation_SEI;
  var $ref_supannEtuDiplome_SEI;
  var $ref_supannEtuEtape_SEI;
  var $ref_supannEtuElementPedagogique_SEI;

  /* chosen value for SEI attribute */
  var $supannEtablissement_SEI;
  //var $supannEtuAnneeInscription_SEI;
  var $supannEtuRegimeInscription_SEI;
  var $supannEtuSecteurDisciplinaire_SEI;
  var $supannEtuTypeDiplome_SEI;
  var $supannEtuCursusAnnee_SEI = 'L1';
  var $supannEntiteAffectation_SEI;
  var $supannEtuDiplome_SEI;
  var $supannEtuEtape_SEI;
  var $supannEtuElementPedagogique_SEI;

  /* supannRoleEntite */
  var $SRE_attributes = array(
    "supannRoleGenerique"         => array('ref'=>TRUE, 'list'=>TRUE, 'shortname'=>"role"),
    "supannTypeEntiteAffectation" => array('ref'=>TRUE, 'list'=>TRUE, 'shortname'=>"type"),
    "supannEntiteAffectation"     => array('ref'=>FALSE,'list'=>TRUE, 'shortname'=>"code"),
  );

  var $ref_supannRoleGenerique_SRE;
  var $ref_supannTypeEntiteAffectation_SRE;

  var $supannRoleGenerique_SRE;
  var $supannTypeEntiteAffectation_SRE;
  var $supannEntiteAffectation_SRE;


  var $single_attributes = /* single valued attributes */
    array("eduPersonPrimaryAffiliation",
          "eduPersonNickName", "eduPersonPrincipalName",
          "supannEntiteAffectationPrincipale",
          "supannAliasLogin",  "supannCivilite",
          "supannListeRouge",
          "supannEmpId",
        );

  var $student_attributes =
    array(
          "supannCodeINE",
          "supannEtuAnneeInscription", "supannEtuSecteurDisciplinaire",
          "supannEtuDiplome", "supannEtuTypeDiplome",
          "supannEtuCursusAnnee", "supannEtuEtape",
          "supannEtuElementPedagogique", "supannEtuRegimeInscription",
          "supannEtuInscription", "supannEtuId",
        );

  var $personal_attributes =
    array(
          "supannEmpCorps", "supannEmpId",
          "supannRoleGenerique", "supannRoleEntite",
          "supannActivite",
        );

  var $ref_attributes =
    array(
            "supannEtablissement",
            "supannTypeEntiteAffectation",
            "supannEmpCorps",
            "supannActivite",
          );

  var $objectclasses = array("eduPerson", "supannPerson");

  function supannAccount(&$config, $dn = NULL)
  {
    global $class_mapping;

    /* on charge la configuration global ede gosa */

    $this->config = $config;

    plugin::plugin($config, $dn);

    $ldap = $this->config->get_ldap_link();

    if ($dn !== NULL) {
      if (isset($this->attrs['objectClass']) &&
          in_array('supannAccount', $this->attrs['objectClass'])) {
        $this->is_account = TRUE;
      }

      /* Save initial account state */
      $this->initially_was_account = $this->is_account;
    }

    foreach($this->attributes as $attr) {
      if (in_array($attr,$this->single_attributes)) {
        if (isset($this->parent->by_object['group']) &&
            isset($this->parent->by_object['group']->$attr)) {
          $this->$attr = &$this->parent->by_object['group']->$attr;
        } elseif(isset($this->attrs[$attr])) {
          $this->$attr = $this->attrs[$attr][0];
        }
      } else {
        $tmp = array();
        if (isset($this->attrs[$attr])) {
          for ($i = 0; $i < $this->attrs[$attr]['count']; $i++) {
            $tmp[$this->attrs[$attr][$i]] = $this->attrs[$attr][$i];
          }
        }
        $this->$attr = $tmp;
      }
      if (empty($this->type_profil) && isset($this->$attr) && !empty($this->$attr)) {
        if (in_array($attr,$this->student_attributes)) {
          $this->type_profil = "student";
        } elseif (in_array($attr,$this->personal_attributes)) {
          $this->type_profil = "personal";
        }
      }
    }

    if (empty($this->type_profil)) {
      $this->type_profil = "student";
    }


    // Reading supann files
    $dir = @opendir(SUPANN_DIR);

    while ($file = readdir($dir)) {
      foreach (array(
                'entites'               => 'supannTypeEntiteAffectation',
                'etablissement'         => 'supannEtablissement',
                'affiliation'           => 'eduPersonAffiliation',
                'activites'             => 'supannActivite',
                'corps'                 => 'supannEmpCorps',
                'roles'                 => 'supannRoleGenerique',
                'discipline'            => 'supannEtuSecteurDisciplinaire',
                'diplome'               => 'supannEtuDiplome',
                'typediplome'           => 'supannEtuTypeDiplome',
                'etuetape'              => 'supannEtuEtape',
                'etuelementpedagogique' => 'supannEtuElementPedagogique',
                'eturegimeinscription'  => 'supannEtuRegimeInscription',
              ) as $shortname => $attr) {
        if (preg_match("/^".$shortname."_/", $file)) {
          $referentiel = preg_replace("/^".$shortname."_/", "", $file);
          $lines = file(SUPANN_DIR.$file);
          $list = array();
          foreach($lines as $line) {
            if (!preg_match("/^#/", $line)) {
              $rec = preg_split('/;/', $line);
              $list[$referentiel][$rec[0]] = $this->truncate_label($rec[1]);
            }
          }
          $list_name = 'list_'.$attr;
          $this->$list_name = $list;
        }
      }
    }

    closedir($dir);

    foreach (array(
                'supannEtuDiplome',
                'supannEtuEtape',
                'supannEtuElementPedagogique',
                ) as $attr) {
      $list_name = 'list_'.$attr;
      $list =& $this->$list_name;
      $list['none']['none'] = _("None");
    }

    /* list of entity stored in LDAP tree */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=supannEntite)",
                  array("supannCodeEntite", "ou", "o"));

    $list_entite = array();
    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['supannCodeEntite'][0])) {
        if (isset($attrs['ou'][0])) {
          $list_entite[] = $attrs['supannCodeEntite'][0].";".$attrs['ou'][0];
        } else {
          $list_entite[] = $attrs['supannCodeEntite'][0].";".$attrs['o'][0];
        }
      }
    }
    $list_entite[] = _(";"._("None"));

    foreach($list_entite as $line) {
      $rec = preg_split('/;/', $line);
      $this->list_supannEntiteAffectation[$rec[0]] = $rec[1]." (".$rec[0].")";
    }

    $this->list_eduPersonAffiliation = $this->list_eduPersonAffiliation['EDU'];

    foreach ($this->ref_attributes as $attr) {
      $ref_name = 'ref_'.$attr;
      $list_name = 'list_'.$attr;
      $refs = array_keys($this->$list_name);
      $this->$ref_name = $refs[0];
    }

    foreach ($this->SEI_attributes as $attr => $flags) {
      if ($flags['ref']) {
        $ref_name = 'ref_'.$attr.'_SEI';
        if (!isset($this->$ref_name)) {
          $list_name = 'list_'.$attr;
          $refs = array_keys($this->$list_name);
          if(in_array('none',$refs)) {
            $this->$ref_name = "none";
          } else {
            $this->$ref_name = $refs[0];
          }
        }
      }
    }

    foreach ($this->SRE_attributes as $attr => $flags) {
      if ($flags['ref']) {
        $ref_name = 'ref_'.$attr.'_SRE';
        if (!isset($this->$ref_name)) {
          $list_name = 'list_'.$attr;
          $refs = array_keys($this->$list_name);
          $this->$ref_name = $refs[0];
        }
      }
    }

    // Labels for the supannEtuCursusAnnee year selection
    $this->cursusAnneeLabels = array(
      _("None"),
      _("1st year"),
      _("2nd year"),
      _("3rd year"),
      _("4th year"),
      _("5th year"),
      _("6th year"),
      _("7th year"),
      _("8th year"),
      _("9th year"),
    );

  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    $display = "";
    /* log function */
    if ($this->is_account && !$this->view_logged) {
      $this->view_logged = TRUE;
      new log("view", "users/".get_class($this), $this->dn);
    }

    if (isset($_POST['modify_state'])) {
      if ($this->is_account && $this->acl_is_removeable()) {
        $this->is_account = FALSE;
      }
      elseif(!$this->is_account && $this->acl_is_createable()) {
        $this->is_account = TRUE;
      }
    }

    if (!$this->is_account && $this->parent == NULL) {
      $display  = "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
                  msgPool::noValidExtension(_("SUPANN"))."</b>";
      $display .= back_to_main();
      return $display;
    }

    /****************
      Account status
     ****************/
    /* Show tab dialog headers */

    if ($this->parent !== NULL) {
      if ($this->is_account) {
        $display = $this->show_disable_header(msgPool::removeFeaturesButton(_("SUPANN")),
                                              msgPool::featuresEnabled(_("SUPANN")));
      } else {
        $display = $this->show_enable_header( msgPool::addFeaturesButton(_("SUPANN")),
                                              msgPool::featuresDisabled(_("SUPANN")));
        return $display;
      }
    }
    /* Templates now! */
    $smarty = get_smarty();

    foreach($this->attributes as $attribute) {
      if (in_array($attribute,$this->single_attributes)) {
        /* assign smarty values for single attribute */
        $smarty->assign($attribute, $this->$attribute);
      } else {
        /* assign smarty values for multiple attribute */
        if (in_array($attribute,$this->ref_attributes)) {
          $list_name = 'list_'.$attribute;
          $array =& $this->$list_name;
          $tmp = array();
          foreach($this->$attribute as $value) {
            $rec = $this->supannExplodeQualifiedAttribute($value);
            $ref = preg_replace('/\{|\}/', '', $rec[0]);

            $info = $array[$ref[0]][$ref[1]];
            $tmp[$value] = $info."(base: ".$ref[0].")";
          }
          $smarty->assign($attribute.'_values', $tmp);

          $ref_name = 'ref_'.$attribute;
          $smarty->assign('list_ref_'.$attribute, array_keys($array));
          $smarty->assign('list_'.$attribute, $array[$this->$ref_name]);
          $smarty->assign($ref_name, $this->$ref_name);
        } elseif (($attribute == 'supannEntiteAffectation') || ($attribute == 'eduPersonAffiliation')) {
          $list_name = 'list_'.$attribute;
          $array =& $this->$list_name;
          $tmp = array();
          foreach($this->$attribute as $attr_value) {
            $tmp[$attr_value] = $array[$attr_value];
          }
          $smarty->assign($attribute.'_values', $tmp);
        } else {
          $tmp = array();
          foreach($this->$attribute as $attr_value) {
            $tmp[$attr_value] = $attr_value;
          }
          $smarty->assign($attribute.'_values', $tmp);
        }
      }
    }

    $profil_list = array("student" => _("student"), "personal" => _("personal"));
    $smarty->assign("profil_list", $profil_list);

    $smarty->assign("type_profil", $this->type_profil);

    $smarty->assign("list_supannCivilite", array("M.", "Mme", "Mlle"));

    if ($this->type_profil == "personal") {
      foreach ($this->SRE_attributes as $SRE_attr => $flags) {
        $list_name = 'list_'.$SRE_attr;
        if ($flags['ref']) {
          $ref_name = 'ref_'.$SRE_attr.'_SRE';
          $array =& $this->$list_name;
          $smarty->assign('list_ref_'.$SRE_attr.'_SRE', array_keys($array));
          $smarty->assign('list_'.$SRE_attr.'_SRE', $array[$this->$ref_name]);
          $smarty->assign($ref_name, $this->$ref_name);
        } elseif ($flags['list']) {
          $smarty->assign('list_'.$SRE_attr.'_SRE', $this->$list_name);
        }
      }

      /* create list box */
      $supannEntiteRoleList = new divSelectBox("supannEntiteRole");
      /* set listbox height */
      $supannEntiteRoleList->SetHeight(100);
      /* store in php value html code for editing entries by POST variable */
      $link_delete =
        "<a href='?plug=".$_GET['plug'].
        "&act=delete_SRE&id=%s'><img style='border: 0px' src='images/lists/trash.png'></a>";

      /* walk all values in supannRoleEntite object */
      foreach($this->supannRoleEntite as $entiteRole => $values) {
        $SRE_values = $this->supannExplodeCompositeAttribute($values);

        $SRE_entry = array();
        foreach ($this->SRE_attributes as $SRE_attr => $flags) {
          if ($flags['ref']) {
            $tmp = $this->supannExplodeQualifiedAttribute($SRE_values[$flags['shortname']]);
            $tmp_value = preg_replace('/\{|\}/', '', $tmp[0]);
            $list_name = 'list_'.$SRE_attr;
            $array =& $this->$list_name;
            $value = $array[$tmp_value[0]][$tmp_value[1]];
          } else {
            $value = $SRE_values[$flags['shortname']];
          }
          $SRE_entry[] = array('string' => _($flags['shortname']).' : '.$value);
        }
        $SRE_entry[] = array('string' => str_replace("%s", base64_encode($entiteRole), $link_delete));

        $supannEntiteRoleList->AddEntry($SRE_entry);
      }

      $smarty->assign("supannEntiteRoleList",
                      $supannEntiteRoleList->DrawList());
    }

    if ($this->type_profil == "student") {
      foreach ($this->SEI_attributes as $SEI_attr => $flags) {
        $list_name = 'list_'.$SEI_attr;
        $ref_name = 'ref_'.$SEI_attr.'_SEI';
        $val_name = $SEI_attr.'_SEI';
        if ($flags['ref']) {
          $array =& $this->$list_name;
          $smarty->assign('list_ref_'.$SEI_attr.'_SEI', array_keys($array));
          $smarty->assign('list_'.$SEI_attr.'_SEI', $array[$this->$ref_name]);
          $smarty->assign($ref_name, $this->$ref_name);
          $smarty->assign($val_name, $this->$val_name);
        } elseif ($flags['list']) {
          $smarty->assign('list_'.$SEI_attr.'_SEI', $this->$list_name);
          $smarty->assign($val_name, $this->$val_name);
        } elseif ($SEI_attr == 'supannEtuCursusAnnee') {
          $smarty->assign('list_'.$SEI_attr.'_SEI', $this->$list_name);
          $cursus_type = substr($this->$val_name,0,1);
          $cursus_year = intval(substr($this->$val_name,1));
          $smarty->assign($ref_name, $cursus_type);
          $smarty->assign($val_name, $cursus_year);
          $smarty->assign("years_supannEtuCursusAnnee",
                          array_slice(
                                      $this->cursusAnneeLabels,
                                      0,
                                      $this->years_supannEtuCursusAnnee[$cursus_type]+1 // we add 1 for the "None" value
                                    )
                        );
        }
      }

      /* create list box */
      $supannEtuInscriptionList = new divSelectBox("supannEtuInscription");
      /* set listbox height */
      $supannEtuInscriptionList->SetHeight(100);
      /* store in php value html code for editing entries by POST variable */
      $link_delete = "<a href='?plug=".$_GET['plug']."&act=delete_SEI&id=%s'><img style='border: 0px' src='images/lists/trash.png'></a>";

      /* walk all values in supannRoleEntite object */
      foreach($this->supannEtuInscription as $entiteSEI => $values) {
        $SEI_values = $this->supannExplodeCompositeAttribute($values);

        $SEI_entry = array();
        foreach ($this->SEI_attributes as $SEI_attr => $flags) {
          if ($flags['ref']) {
            if (isset($SEI_values[$flags['shortname']])) {
              $tmp = $this->supannExplodeQualifiedAttribute($SEI_values[$flags['shortname']]);
              $tmp_value = preg_replace('/\{|\}/', '', $tmp[0]);
              $list_name = 'list_'.$SEI_attr;
              $array =& $this->$list_name;
              $value = $array[$tmp_value[0]][$tmp_value[1]];
            } else {
              $value = "";
            }
          } elseif ($SEI_attr == 'supannEtuCursusAnnee') {
            $tmp = $SEI_values[$flags['shortname']];
            $list_name = 'list_'.$SEI_attr;
            $array =& $this->$list_name;

            $type_cursus = substr($tmp, 0, 1);
            $reste_cursus = intval(substr($tmp, 1));
            $value = $array[$type_cursus];
            if ($reste_cursus != 0) {
              $value .= " (".$this->cursusAnneeLabels[$reste_cursus].")";
            }
          } else {
            if (isset($SEI_values[$flags['shortname']])) {
              $value = $SEI_values[$flags['shortname']];
            } else {
              $value = "";
            }
          }
          $SEI_entry[] = array('string' => _($flags['shortname']).' : '.$value);
        }
        $SEI_entry[] = array('string' => str_replace("%s", base64_encode($entiteSEI), $link_delete));

        $supannEtuInscriptionList->AddEntry($SEI_entry);
      }
      $smarty->assign("supannEtuInscriptionList", $supannEtuInscriptionList->DrawList());
    }

    $smarty->assign("supannListeRouge_flag", ($this->supannListeRouge == "TRUE"));

    $smarty->assign("list_supannEntiteAffectation", $this->list_supannEntiteAffectation);
    $smarty->assign("list_eduPersonAffiliation",    $this->list_eduPersonAffiliation);

    $smarty->assign("HELP_eduPersonPrincipalName",  _("HELP_eduPersonPrincipalName"));
    $smarty->assign("HELP_supannAutreTelephone",    _("HELP_supannAutreTelephone"));
    $smarty->assign("HELP_supannAliasLogin",        _("HELP_supannAliasLogin"));
    $smarty->assign("HELP_supannCivilite",          _("HELP_supannCivilite"));
    $smarty->assign("HELP_supannListeRouge",        _("HELP_supannListeRouge"));
    $smarty->assign("HELP_supannAutreMail",         _("HELP_supannAutreMail"));
    $smarty->assign("HELP_supannMailPerso",         _("HELP_supannMailPerso"));
    $smarty->assign("HELP_supannEmpId",             _("HELP_supannEmpId"));
    $smarty->assign("HELP_eduPersonAffiliation",    _("HELP_eduPersonAffiliation"));
    $smarty->assign("HELP_supannEmpCorps",          _("HELP_supannEmpCorps"));
    $smarty->assign("HELP_supannActivite",          _("HELP_supannActivite"));
    $smarty->assign("HELP_supannEntiteAffectationPrincipale", _("HELP_supannEntiteAffectationPrincipale"));
    $smarty->assign("HELP_supannEntiteAffectation", _("HELP_supannEntiteAffectation"));
    $smarty->assign("HELP_supannEtablissement",     _("HELP_supannEtablissement"));
    $smarty->assign("HELP_supannTypeEntiteAffectation", _("HELP_supannTypeEntiteAffectation"));
    $smarty->assign("HELP_supannRoleGenerique",     _("HELP_supannRoleGenerique"));
    $smarty->assign("HELP_supannRoleEntite",        _("HELP_supannRoleEntite"));

    $display = $display.$smarty->fetch(get_template_path('supann.tpl', TRUE, dirname(__FILE__)));

    return $display;
  }

  /* Validate user inputthis->$attr
   */
  function check()
  {
    $message = plugin::check();
    if (($this->type_profil != "student") && ($this->type_profil != "personal")) {
      $message[] = msgPool::invalid("type_profil");
    }

    return $message;
  }

  /* Save data to object */
  function save_object()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "dn");
    /* Save additional values for possible next step */
    if (isset($_GET['act']) && $_GET['act'] == "delete_SRE") {
      $tmp = base64_decode($_GET['id']);
      unset($this->supannRoleEntite[$tmp]);
    }
    if (isset($_GET['act']) && $_GET['act'] == "delete_SEI") {
      $tmp = base64_decode($_GET['id']);
      unset($this->supannEtuInscription[$tmp]);
    }
    if (isset($_POST['supannAccountTab'])) {
      if (isset($_POST['type_profil']) && $_POST['type_profil'] == "student") {
        $this->type_profil = "student";
      } else {
        $this->type_profil = "personal";
      }

      plugin::save_object();

      if (isset($_POST['supannListeRouge'])) {
        $this->supannListeRouge = "TRUE";
      } else {
        $this->supannListeRouge = "FALSE";
      }

      foreach(array("supannAutreMail","supannMailPerso","supannAutreTelephone",
                    "supannEntiteAffectation","eduPersonAffiliation",
                    "supannCodeINE","supannEtuAnneeInscription","supannEtuId","supannEtuInscription")
              as $attr) {
        /* add Value */
        if (isset($_POST[$attr.'_new']) && isset($_POST['add'.$attr])) {
          $array =& $this->$attr;
          $array[$_POST[$attr.'_new']] = $_POST[$attr.'_new'];
        }
        /* delete Value */
        if (isset($_POST['del'.$attr]) && isset($_POST[$attr.'_selected'])) {
          foreach($_POST[$attr.'_selected'] as $name) {
            $array =& $this->$attr;
            unset($array[$name]);
          }
        }
      }

      foreach($this->ref_attributes as $attr) {
        /* add Value */
        if (isset($_POST[$attr.'_new']) && isset($_POST['add'.$attr])) {
          $rec = "{".$_POST['ref_'.$attr]."}".$_POST[$attr.'_new'];
          $array =& $this->$attr;
          $array[$rec] = $rec;
        }
        /* delete Value */
        if (isset($_POST['del'.$attr]) && isset($_POST[$attr.'_selected'])) {
          foreach($_POST[$attr.'_selected'] as $name) {
            $array =& $this->$attr;
            unset($array[$name]);
          }
        }
        $ref_name = 'ref_'.$attr;
        if (isset($_POST[$ref_name])) {
          $this->$ref_name = $_POST[$ref_name];
        }
      }

      if ($this->type_profil == 'personal') {
        /* supannRoleEntite */
        foreach ($this->SRE_attributes as $SRE_attr => $flags) {
          if ($flags['ref']) {
            $ref_name = 'ref_'.$SRE_attr.'_SRE';
            if (isset($_POST[$ref_name])) {
              $this->$ref_name = $_POST[$ref_name];
            }
          }
          $val_name = $SRE_attr.'_SRE';
          if (isset($_POST[$val_name])) {
            $this->$val_name = $_POST[$val_name];
          }
        }

        if (isset($_POST['addsupannRoleEntite'])) {
          $rec = "";
          foreach ($this->SRE_attributes as $SRE_attr => $flags) {
            $val_name = $SRE_attr.'_SRE';
            $value = $this->$val_name;
            if ($flags['ref']) {
              $ref_name = 'ref_'.$SRE_attr.'_SRE';
              $value    = "{".$this->$ref_name."}".$value;
            }
            $values =& $this->$SRE_attr;
            if (!isset($values[$this->$val_name])) {
              /* Adding the value to the attribute itself */
              $values[$value] = $value;
            }
            /* SRE entry */
            $rec .= "[".$flags['shortname']."=".$value."]";
          }
          $this->supannRoleEntite[$rec] = $rec;
        }
      } else {
        /* supannEtuInscription */
        foreach ($this->SEI_attributes as $SEI_attr => $flags) {
          if ($flags['ref']) {
            $ref_name = 'ref_'.$SEI_attr.'_SEI';
            if (isset($_POST[$ref_name])) {
              $this->$ref_name = $_POST[$ref_name];
            }
          }
          $val_name = $SEI_attr.'_SEI';
          if (isset($_POST[$val_name])) {
            if ($SEI_attr == 'supannEtuCursusAnnee') {
              $year = intval($_POST[$val_name]);
              if ($year <= 0) {
                $year = "";
              }
              $this->$val_name = $_POST['ref_'.$val_name].$year;
            } else {
              $this->$val_name = $_POST[$val_name];
            }
          }
        }

        if (isset($_POST['addsupannEtuInscription'])) {
          $rec = "";
          foreach ($this->SEI_attributes as $SEI_attr => $flags) {
            $val_name = $SEI_attr.'_SEI';
            $value = $this->$val_name;
            if ($flags['ref']) {
              $ref_name = 'ref_'.$SEI_attr.'_SEI';
              if ($this->$ref_name == 'none') {
                continue;
              }
              $value = "{".$this->$ref_name."}".$value;
            }
            if ($value == "") {
              continue;
            }
            $values =& $this->$SEI_attr;
            if (!isset($values[$this->$val_name])) {
              /* Adding the value to the attribute itself */
              $values[$value] = $value;
            }
            /* SEI entry */
            $rec .= "[".$flags['shortname']."=".$value."]";
          }
          $this->supannEtuInscription[$rec] = $rec;
        }
      }
    }
  }

  /* Save attribute modifications back to the ldap
   */
  function save()
  {
    if ($this->type_profil == "student") {
      foreach ($this->personal_attributes as $attribute) {
        $this->$attribute = "";
      }
    } elseif ($this->type_profil == "personal") {
      foreach ($this->student_attributes as $attribute) {
        $this->$attribute = "";
      }
    }

    foreach($this->attributes as $attr) {
      if (is_array($this->$attr)) {
        $tmp = array();
        foreach($this->$attr as $item) {
          array_push($tmp,$item);
        }
        $this->$attr = $tmp;
      }
    }

    /* Save values to object */
    plugin::save();

    $this->cleanup();

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    $ldap->modify($this->attrs);
    if (!$ldap->success()) {
      msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class());
    }
  }

  /*! \brief  Removes the mailAccount extension from ldap
   */
  function remove_from_parent()
  {
    /* Cancel if there's nothing to do here */
    if (!$this->initially_was_account) {
      return;
    }

    /* Remove GOsa attributes */
    plugin::remove_from_parent();


    @DEBUG(DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__, $this->attributes, "Save");
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    $ldap->modify($this->attrs);

    /* Add "view" to logging class */
    new log("remove", "users/".get_class($this), $this->dn,
            array_keys($this->attrs), $ldap->get_error());
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"),
                          msgPool::ldaperror($ldap->get_error(), $this->dn,
                                             LDAP_MOD, get_class()));
    }

    /* Optionally execute a command after we're done */
    $this->handle_post_events("remove", array("uid" => $this->uid));
  }

  static function plInfo()
  {
    return array("plShortName" => _("SUPANN Informations"),
                 "plDescription" => _("SUPANN informatiosn management plugin"),
                 "plSelfModify" => TRUE,
                 "plDepends" => array("users"),
                 "plPriority" => 5,
                 "plSection" => array("personal" => _("My account")),
                 "plCategory" => array("users"),
                 "plOptions" => array(),
                 "plProvidedAcls" => array(
                    "eduPersonAffiliation" => _("eduPersonAffiliation"),
                    "eduPersonPrimaryAffiliation" => _("eduPersonPrimaryAffiliation"),
                    "eduPersonNickName" => _("eduPersonNickName"),
                    "eduPersonPrincipalName" => _("eduPersonPrincipalName"),
                    "supannEntiteAffectation" => _("supannEntiteAffectation"),
                    "supannEntiteAffectationPrincipale" => _("supannEntiteAffectationPrincipale"),
                    "supannAliasLogin" => _("supannAliasLogin"),
                    "supannAutreMail" => _("supannAutreMail"),
                    "supannAutreTelephone" => _("supannAutreTelephone"),
                    "supannCivilite" => _("supannCivilite"),
                    "supannEtablissement" => _("supannEtablissement"),
                    "supannListeRouge" => _("supannListeRouge"),
                    "supannMailPerso" => _("supannMailPerso"),
                    "supannTypeEntiteAffectation" => _("supannTypeEntiteAffectation"),
                    "supannCodeINE" => _("supannCodeINE"),
                    "supannEtuId" => _("supannEtuId"),
                    "supannEtuInscription" => _("supannEtuInscription"),
                    "supannEtuAnneeInscription" => _("supannEtuAnneeInscription"),
                    "supannEtuRegimeInscription" => _("supannEtuRegimeInscription"),
                    "supannEtuSecteurDisciplinaire" => _("supannEtuSecteurDisciplinaire"),
                    "supannEtuTypeDiplome" => _("supannEtuTypeDiplome"),
                    "supannEtuCursusAnnee" => _("supannEtuCursusAnnee"),
                    "supannEtuDiplome" => _("supannEtuDiplome"),
                    "supannEtuEtape" => _("supannEtuEtape"),
                    "supannEtuElementPedagogique" => _("supannEtuElementPedagogique"),
                    "supannEmpCorps" => _("supannEmpCorps"),
                    "supannEmpId" => _("supannEmpId"),
                    "supannRoleGenerique" => _("supannRoleGenerique"),
                    "supannRoleEntite" => _("supannRoleEntite"),
                    "supannActivite" => _("supannActivite"),
                  )
                );
  }

  function supannExplodeQualifiedAttribute($val)
  {
    $accolade_pattern = '#\{.*?\}|[^ ]+#';
    preg_match_all($accolade_pattern, $val, $tabResult);
    return ($tabResult);
  }

  function supannExplodeCompositeAttribute($str)
  {
    preg_match_all('`\[([^\]]*)\]`', $str, $tabResult);
    for ($i = 0; $i < count($tabResult[1]); $i++) {
      $val = preg_split('/\=/i', $tabResult[1][$i]);
      $res[$val[0]] = $val[1];
    }

    return $res;
  }

  /* return the 64 first chars and "…" after if text is longer */
  function truncate_label($str,$len = 64)
  {
    if (strlen($str) > $len) {
      return substr($str,0,$len)."…";
    } else {
      return $str;
    }
  }
}
?>
