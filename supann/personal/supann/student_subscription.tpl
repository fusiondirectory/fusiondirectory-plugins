<fieldset id="{$sectionId}" class="plugin_section{$sectionClasses}">
  <legend><span><label for="{$attributes.supannEtuInscription.htmlid}">{$section}</label></span></legend>
  <div>
  <table>
    <tr>
      <td title="{$attributes.supannEtuInscription.description}" colspan="4">
        {eval var=$attributes.supannEtuInscription.input}
      </td>
    </tr>
    <tr>
      <td title="{$attributes.supannEtablissement_etab.description}">
        <label for="{$attributes.supannEtablissement_etab.htmlid}">
          {eval var=$attributes.supannEtablissement_etab.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtablissement_etab.input}</td>
      <td title="{$attributes.supannEtuAnneeInscription_anneeinsc.description}">
        <label for="{$attributes.supannEtuAnneeInscription_anneeinsc.htmlid}">
          {eval var=$attributes.supannEtuAnneeInscription_anneeinsc.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuAnneeInscription_anneeinsc.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.supannEtuRegimeInscription_regimeinsc.description}">
        <label for="{$attributes.supannEtuRegimeInscription_regimeinsc.htmlid}">
          {eval var=$attributes.supannEtuRegimeInscription_regimeinsc.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuRegimeInscription_regimeinsc.input}</td>
      <td title="{$attributes.supannEtuSecteurDisciplinaire_sectdisc.description}">
        <label for="{$attributes.supannEtuSecteurDisciplinaire_sectdisc.htmlid}">
          {eval var=$attributes.supannEtuSecteurDisciplinaire_sectdisc.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuSecteurDisciplinaire_sectdisc.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.supannEtuTypeDiplome_typedip.description}">
        <label for="{$attributes.supannEtuTypeDiplome_typedip.htmlid}">
          {eval var=$attributes.supannEtuTypeDiplome_typedip.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuTypeDiplome_typedip.input}</td>
      <td title="{$attributes.supannEtuCursusAnnee_cursusann.description}">
        <label for="{$attributes.supannEtuCursusAnnee_cursusann.htmlid}">
          {eval var=$attributes.supannEtuCursusAnnee_cursusann.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuCursusAnnee_cursusann.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.supannEntiteAffectation_affect.description}">
        <label for="{$attributes.supannEntiteAffectation_affect.htmlid}">
          {eval var=$attributes.supannEntiteAffectation_affect.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEntiteAffectation_affect.input}</td>
      <td title="{$attributes.supannEtuDiplome_diplome.description}">
        <label for="{$attributes.supannEtuDiplome_diplome.htmlid}">
          {eval var=$attributes.supannEtuDiplome_diplome.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuDiplome_diplome.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.supannEtuEtape_etape.description}">
        <label for="{$attributes.supannEtuEtape_etape.htmlid}">
          {eval var=$attributes.supannEtuEtape_etape.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuEtape_etape.input}</td>
      <td title="{$attributes.supannEtuElementPedagogique_eltpedago.description}">
        <label for="{$attributes.supannEtuElementPedagogique_eltpedago.htmlid}">
          {eval var=$attributes.supannEtuElementPedagogique_eltpedago.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuElementPedagogique_eltpedago.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.supannEtuInscription_buttons.description}" colspan="4">
        {eval var=$attributes.supannEtuInscription_buttons.input}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
