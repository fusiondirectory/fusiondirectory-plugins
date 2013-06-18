<script src="include/overlib.js" type="text/javascript"></script>

<table style="text-align: left; width: 100%;" border="0" cellpadding="2" cellspacing="2">
  <tbody>
    <tr>
      <td colspan="6" rowspan="1" style="vertical-align: top;">
        <h2><img class="center" alt="" src="plugins/quota/images/iconMini.png"> {t}Quota service Properties{/t}</h2>
      </td>
    </tr>
    <tr>
      <td colspan="6" style="vertical-align: top;">
        <h3><img class="center" alt="" src="plugins/quota/images/iconMini.png"> {t}quota device{/t}</h3>
      </td>
    </tr>
    <tr>
      <td colspan="6" style="vertical-align: top;">
        {$quotaDevicesList}
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top;">
        {t}device{/t}<br>
      </td>
      <td style="vertical-align: top;">
        <!-- <input name="quota_device_name" id="quota_device_name" type="text">-->
        <select id="quota_device_name" size="1" name="quota_device_name" >
          {html_options options=$shareInfos}
        </select>

      </td>
      <td style="vertical-align: top;">
        {t}blocksize{/t}<br>
      </td>
      <td style="vertical-align: top;">
        <input name="quota_device_blocksize" id="quota_device_blocksize" type="text">
      </td>

    </tr>
    <tr>
      <td style="vertical-align: top;">
        {t}description{/t}
      </td>
      <td colspan="5" style="vertical-align: top;">
        <input name="quota_device_description" id="quota_device_description" size="70" type="text" >
      </td>
    </tr>
    <tr>
      <td colspan="6" style="vertical-align: top; ">
        <input name="addquotadevice" value="{msgPool type=addButton}" id="addquotadevice" type="submit">

      </td>
    </tr>
    <tr>
      <td colspan="6"style="vertical-align: top;">
        <p class="seperator">&nbsp;</p>
      </td>
    </tr>
    <tr>
      <td colspan="6" style="vertical-align: top;">
        <h3><img class="center" alt="" src="plugins/quota/images/warning.png"> {t}Quota Warning Message{/t}</h3>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top;">
        <label for="quotaMsgCharsetSupport">{t}Charset{/t}</label>
      </td>
      <td style="vertical-align: top;">
        <select size="1" id="quotaMsgCharsetSupport" name="quotaMsgCharsetSupport">
          {html_options options=$charsets selected=$quotaMsgCharsetSupport}
        </select>
      </td>
      <td style="vertical-align: top;">
        <label for="quotaMailCommand">{t}Mail Command{/t}</label>
      </td>
      <td style="vertical-align: top;">
        <input name="quotaMailCommand" id="quotaMailCommand" size="25" type="text" value="{$quotaMailCommand}">
      </td>

    </tr>
    <tr>
      <td style="vertical-align: top;">
        <label for="quotaMsgContactSupport">{t}Support Contact{/t}</label>
      </td>
      <td style="vertical-align: top;">
        <input name="quotaMsgContactSupport" id="quotaMsgContactSupport" size="25" type="text"  value="{$quotaMsgContactSupport}">
      </td>
      <td style="vertical-align: top;">
        <label for="quotaMsgFromSupport">{t}From{/t}</label>
      </td>
      <td style="vertical-align: top;">
        <input name="quotaMsgFromSupport" id="quotaMsgFromSupport" type="text" value="{$quotaMsgFromSupport}">
      </td>
      <td style="vertical-align: top;">
        <label for="quotaCarbonCopyMail">{t}CC{/t}</label>
      </td>
      <td style="vertical-align: top;">
        <input name="quotaCarbonCopyMail" id="quotaCarbonCopyMail" size="25" type="text" value="{$quotaCarbonCopyMail}">
      </td>

    </tr>
    <tr>
      <td style="vertical-align: top;">
        <label for="quotaMsgSubjectSupport">{t}Subject{/t}</label>
      </td>
      <td colspan="6" style="vertical-align: top;">
        <input name="quotaMsgSubjectSupport" id="quotaMsgSubjectSupport" size="60" type="text" value="{$quotaMsgSubjectSupport}" >
      </td>

    </tr>
    <tr>
      <td style="vertical-align: top;">
        <label for="quotaMsgContentSupport">{t}Content{/t}</label>
      </td>
      <td colspan="5" style="vertical-align: top;">
        <textarea input name="quotaMsgContentSupport" id="quotaMsgContentSupport"   cols="100" rows="5">{$quotaMsgContentSupport}</textarea>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top;">
        <label for="quotaMsgSignatureSupport">{t}Signature{/t}</label>
      </td>
      <td style="vertical-align: top;">
        <input name="quotaMsgSignatureSupport" id="quotaMsgSignatureSupport" size="25" type="text" value="{$quotaMsgSignatureSupport}">
      </td>
    </tr>
    <tr>
      <td colspan="5" style="vertical-align: top;">
        <h3><img class="center" alt="" src="plugins/quota/images/ldap.png"> {t}LDAP Message Support{/t}</h3>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top;">
        <label for="quotaLdapServerURI">{t}Ldap server{/t}</label>
      </td>
      <td style="vertical-align: top;">
        <select id="quotaLdapServerURI" size="1" name="quotaLdapServerURI" size="10">
          {html_options options=$ldapServers selected=$quotaLdapServerURI}
        </select>
       </td>
    </tr>
    <tr>
      <td style="vertical-align: top;">
        <label for="quotaLdapServerUserDn">{t}Ldap User DN{/t}</label>
      </td>
      <td style="vertical-align: top;">
        <input name="quotaLdapServerUserDn" id="quotaLdapServerUserDn" type="text" value="{$quotaLdapServerUserDn}" />
      </td>
      <td style="vertical-align: top;">
        <label for="quotaLdapServerUserPassword">{t}Ldap User Password{/t}</label>
      </td>
      <td style="vertical-align: top;">
        <input name="quotaLdapServerUserPassword" id="quotaLdapServerUserPassword" type="password" value="{$quotaLdapServerUserPassword}" />
      </td>
    </tr>
  </tbody>
</table>

<input type="hidden" name="serviceQuotaPosted" value="1">

<p class="seperator">&nbsp;</p>
<div style="width: 100%; text-align: right; padding-top: 10px; padding-bottom: 3px;">
<input name="SaveService" value="{msgPool type=saveButton}" type="submit"> &nbsp; <input name="CancelService" value="{msgPool type=cancelButton}" type="submit"> </div>
