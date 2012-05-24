<table style="text-align: left; width: 100%;" border="0" cellpadding="2"cellspacing="2">
  <tbody>
    <tr>
      <td colspan="6" rowspan="1" style="vertical-align: top;">
      <h1><img class="center" alt="" src="plugins/quota/images/storage.png" align="middle"> {t}Quota Informations{/t}<h1>
      </td>
    </tr>
    <tr>
      <td colspan="6" rowspan="1" style="width:100%;vertical-align:top;">
        <h2>{t}Quota{/t}</h2>
          {$QuotaList}
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top;">
        {t}Server{/t}
      </td>
      <td>
        <select id="" size="1" name="quotaServer" size="10" onchange="document.mainform.submit()">
          {html_options options=$quotaServersList selected=$quotaServer}
        </select>
      </td>
      <td style="vertical-align: top;">
        {t}Device{/t}
      </td>
      <td style="vertical-align: top;">
        <select id="" size="1" name="quotaDevice" size="10">
          {html_options options=$quotaDeviceList}
        </select>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top;">
        {t}Block soft limit{/t}
      </td>
      <td style="vertical-align: top;">
        <input id="quota_blocksoft" name="quota_blocksoft" size="20" maxlength="10" type="text" value="{$quota_blocksoft}" />
        <select name="quota_blocksoft_unit" >
          {html_options options=$unit_options selected=$quota_blocksoft_unit}
        </select>
      </td>
      <td style="vertical-align: top;">
        {t}Block hard limit{/t}
      </td>
      <td style="vertical-align: top;">
        <input id="quota_blockhard" name="quota_blockhard" size="20" maxlength="10" type="text" value="{$quota_blockhard}" />
        <select name="quota_blockhard_unit" >
          {html_options options=$unit_options selected=$quota_blockhard_unit}
        </select>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top;">
        {t}Inode soft limit{/t}
      </td>
      <td style="vertical-align: top;">
        <input id="quota_inodesoft" name="quota_inodesoft" size="20" maxlength="10" type="text" value="{$quota_inodesoft}" />
      </td>
      <td style="vertical-align: top;">
        {t}Inode hard limit{/t}
      </td>
      <td style="vertical-align: top;">
        <input id="quota_inodehard" name="quota_inodehard" size="20" maxlength="10" type="text" value="{$quota_inodesoft}" />
      </td>
    </tr>
    <tr>
      <td colspan="6" rowspan="1" style="vertical-align: top;">
        <input type="submit" name="addquota" value="{msgPool type=addButton}" id="addquota">
      </td>
    </tr>
  </tbody>
</table>


<input type="hidden" name="quotaTab" value="quotaTab">
