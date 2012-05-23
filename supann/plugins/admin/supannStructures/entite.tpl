<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=4>
  <tbody>
    <tr>
      <td colspan="4" rowspan="1"><h2>{t}Entite{/t}</h2></td>
    </tr>
    <tr>
      <td><label for="ou">{t}OU name{/t}{$must}</label><br> </td>
      <td><input type="text" name="ou" id="ou" value="{$ou}" size="35"></td>
      <td></td>
      <td></td>
    </tr>
    <tr>
      <td colspan="2" rowspan="1"><h3>{t}Administrative Information{/t}</h3></td>
      <td colspan="2" rowspan="1"><h3>{t}Supann Informations{/t}</h3></td>
    </tr>
    <tr>
      <td><label for="telephoneNumber">{t}Telephone{/t}</label></td>
      <td><input type="text" name="telephoneNumber" id="telephoneNumber" value="{$telephoneNumber}" size="35"></td>
      <td><label for="supannTypeEntite">{t}Type entite Supann{/t}</label></td>
      <td>
        <select id="supannTypeEntite" size="1" name="supannTypeEntite">
          {html_options options=$typesEntites selected=$supannTypeEntite}
        </select>
        <br>
      </td>
    </tr>
    <tr>
      <td><label for="facsimileTelephoneNumber">{t}Fax{/t}</label></td>
      <td><input type="text" name="facsimileTelephoneNumber" id="facsimileTelephoneNumber" value="{$facsimileTelephoneNumber}" size="35"></td>
      <td><label for="supannCodeEntite">{t}Supann entity code{/t}{$must}</label></td>
      <td><input type="text" name="supannCodeEntite" id="supannCodeEntite" value="{$supannCodeEntite}" size="35"></td>
    </tr>
    <tr>
      <td><label for="postalAddress">{t}Postal Address{/t}</label></td>
      <td><input type="text" name="postalAddress" id="postalAddress" value="{$postalAddress}" size="35"></td>
      <td><label for="supannCodeEntiteParent">{t}Supann parent entity code{/t}</label></td>
      <td>
      <select id="supannCodeEntiteParent" size="1" name="supannCodeEntiteParent">
        {html_options options=$supannEntiteParentList selected=$supannCodeEntiteParent}
      </select>
    <tr>
      <td><label for="description">{t}Description{/t}</label></td>
      <td><input type="text" name="description" id="description" value="{$description}" size="35"></td>
      <td><label for="supannRefId">{t}Supann Reference ID{/t}</label></td>
      <td><input type="text" name="supannRefId" id="supannRefId" value="{$supannRefId}" size="35"></td>
    </tr>
  </tbody>
</table>

<p class='seperator'>&nbsp;</p>


<!-- Place cursor -->
<input type='hidden' name='entite_posted' value='1'>
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
  focus_field('ou');
  -->
</script>
