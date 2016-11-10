<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span>{$section}</span></legend>
  <div>
    <table>
      <tr>
        <td title="{$attributes.gosaProxyAcctFlags.description}"><label for="gosaProxyAcctFlags">{eval var=$attributes.gosaProxyAcctFlags.label}</label></td>
        <td>{eval var=$attributes.gosaProxyAcctFlags.input}</td>
      </tr>
      <tr>
        <td title="{$attributes.gosaProxyWorkingStart.description}">
          <label for="enableWorkingTime">{eval var=$attributes.gosaProxyWorkingStart.label}</label>
        </td>
        <td>{eval var=$attributes.enableWorkingTime.input} {eval var=$attributes.gosaProxyWorkingStart.input} - {eval var=$attributes.gosaProxyWorkingStop.input}</td>
      </tr>
      <tr>
        <td title="{$attributes.gosaProxyQuota.description}">
          <label for="enableQuota">{eval var=$attributes.gosaProxyQuota.label}</label>
        </td>
        <td>{eval var=$attributes.enableQuota.input} {eval var=$attributes.gosaProxyQuota.input} <label for="gosaProxyQuotaPeriod">{eval var=$attributes.gosaProxyQuotaPeriod.label}</label> {eval var=$attributes.gosaProxyQuotaPeriod.input}</td>
      </tr>
    </table>
  </div>
</fieldset>
