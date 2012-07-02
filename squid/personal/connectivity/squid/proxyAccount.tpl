<fieldset id="{$sectionId}" class="proxyAccount">
  <legend>{$section}</legend>
  <table>
      <tr>
        <td title="{$attributes.gosaProxyAcctFlags.description}"><label for="gosaProxyAcctFlags">{$attributes.gosaProxyAcctFlags.label}</label></td>
        <td>{eval var=$attributes.gosaProxyAcctFlags.input}</td>
      </tr>
      <tr>
        <td title="{$attributes.gosaProxyWorkingStart.description}">
          <label for="enableWorkingTime">{$attributes.gosaProxyWorkingStart.label}</label>
        </td>
        <td>{eval var=$attributes.enableWorkingTime.input} {eval var=$attributes.gosaProxyWorkingStart.input} - {eval var=$attributes.gosaProxyWorkingStop.input}</td>
      </tr>
  	  <tr>
  	    <td title="{$attributes.gosaProxyQuota.description}">
  	      <label for="enableQuota">{$attributes.gosaProxyQuota.label}</label>
  	    </td>
  	    <td>{eval var=$attributes.enableQuota.input} {eval var=$attributes.gosaProxyQuota.input} <label for="gosaProxyQuotaPeriod">{$attributes.gosaProxyQuotaPeriod.label}</label> {eval var=$attributes.gosaProxyQuotaPeriod.input}</td>
  	  </tr>
  </table>
</fieldset>
