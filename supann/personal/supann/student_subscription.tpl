<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span><label for="{$attributes.supannEtuInscription.htmlid}">{if !empty($sectionIcon)}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}</label></span></legend>
  <div>
  <table>
    <tr class="
      {if $attributes.supannEtuInscription.subattribute}subattribute{/if}
      {if $attributes.supannEtuInscription.required}required{/if}
      {if !$attributes.supannEtuInscription.readable}nonreadable{/if}
      {if !$attributes.supannEtuInscription.writable}nonwritable{/if}
    ">
      <td title="{$attributes.supannEtuInscription.description|escape}" colspan="4">
        {eval var=$attributes.supannEtuInscription.input}
      </td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.supannEtuInscription.readable}nonreadable{/if}
      {if !$attributes.supannEtuInscription.writable}nonwritable{/if}
    ">
      <td title="{$attributes.supannEtablissement_etab.description|escape}">
        <label for="{$attributes.supannEtablissement_etab.htmlid}">
          {eval var=$attributes.supannEtablissement_etab.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtablissement_etab.input}</td>
      <td title="{$attributes.supannEtuAnneeInscription_anneeinsc.description|escape}">
        <label for="{$attributes.supannEtuAnneeInscription_anneeinsc.htmlid}">
          {eval var=$attributes.supannEtuAnneeInscription_anneeinsc.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuAnneeInscription_anneeinsc.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.supannEtuInscription.readable}nonreadable{/if}
      {if !$attributes.supannEtuInscription.writable}nonwritable{/if}
    ">
      <td title="{$attributes.supannEtuRegimeInscription_regimeinsc.description|escape}">
        <label for="{$attributes.supannEtuRegimeInscription_regimeinsc.htmlid}">
          {eval var=$attributes.supannEtuRegimeInscription_regimeinsc.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuRegimeInscription_regimeinsc.input}</td>
      <td title="{$attributes.supannEtuSecteurDisciplinaire_sectdisc.description|escape}">
        <label for="{$attributes.supannEtuSecteurDisciplinaire_sectdisc.htmlid}">
          {eval var=$attributes.supannEtuSecteurDisciplinaire_sectdisc.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuSecteurDisciplinaire_sectdisc.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.supannEtuInscription.readable}nonreadable{/if}
      {if !$attributes.supannEtuInscription.writable}nonwritable{/if}
    ">
      <td title="{$attributes.supannEtuTypeDiplome_typedip.description|escape}">
        <label for="{$attributes.supannEtuTypeDiplome_typedip.htmlid}">
          {eval var=$attributes.supannEtuTypeDiplome_typedip.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuTypeDiplome_typedip.input}</td>
      <td title="{$attributes.supannEtuCursusAnnee_cursusann.description|escape}">
        <label for="{$attributes.supannEtuCursusAnnee_cursusann.htmlid}">
          {eval var=$attributes.supannEtuCursusAnnee_cursusann.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuCursusAnnee_cursusann.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.supannEtuInscription.readable}nonreadable{/if}
      {if !$attributes.supannEtuInscription.writable}nonwritable{/if}
    ">
      <td title="{$attributes.supannEntiteAffectation_affect.description|escape}">
        <label for="{$attributes.supannEntiteAffectation_affect.htmlid}">
          {eval var=$attributes.supannEntiteAffectation_affect.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEntiteAffectation_affect.input}</td>
      <td title="{$attributes.supannEtuDiplome_diplome.description|escape}">
        <label for="{$attributes.supannEtuDiplome_diplome.htmlid}">
          {eval var=$attributes.supannEtuDiplome_diplome.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuDiplome_diplome.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.supannEtuInscription.readable}nonreadable{/if}
      {if !$attributes.supannEtuInscription.writable}nonwritable{/if}
    ">
      <td title="{$attributes.supannEtuEtape_etape.description|escape}">
        <label for="{$attributes.supannEtuEtape_etape.htmlid}">
          {eval var=$attributes.supannEtuEtape_etape.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuEtape_etape.input}</td>
      <td title="{$attributes.supannEtuElementPedagogique_eltpedago.description|escape}">
        <label for="{$attributes.supannEtuElementPedagogique_eltpedago.htmlid}">
          {eval var=$attributes.supannEtuElementPedagogique_eltpedago.label}
        </label>
      </td>
      <td>{eval var=$attributes.supannEtuElementPedagogique_eltpedago.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.supannEtuInscription.readable}nonreadable{/if}
      {if !$attributes.supannEtuInscription.writable}nonwritable{/if}
    ">
      <td title="{$attributes.supannEtuInscription_buttons.description|escape}" colspan="4">
        {eval var=$attributes.supannEtuInscription_buttons.input}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
