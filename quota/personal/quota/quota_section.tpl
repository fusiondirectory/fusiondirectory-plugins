<fieldset id="{$sectionId}" class="plugin_section{$sectionClasses}">
  <legend><span><label for="{$attributes.quota.htmlid}">{$section}</label></span></legend>
  <div>
  <table>
    <tr>
      <td title="{$attributes.quota.description}" colspan="4">
        {eval var=$attributes.quota.input}
      </td>
    </tr>
    <tr>
      <td title="{$attributes.quotaServer.description}">
        <label for="{$attributes.quotaServer.htmlid}">
          {eval var=$attributes.quotaServer.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaServer.input}</td>
      <td title="{$attributes.quotaDevice.description}">
        <label for="{$attributes.quotaDevice.htmlid}">
          {eval var=$attributes.quotaDevice.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaDevice.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.quotaBlockSoftLimit.description}">
        <label for="{$attributes.quotaBlockSoftLimit.htmlid}">
          {eval var=$attributes.quotaBlockSoftLimit.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaBlockSoftLimit.input}</td>
      <td title="{$attributes.quotaBlockHardLimit.description}">
        <label for="{$attributes.quotaBlockHardLimit.htmlid}">
          {eval var=$attributes.quotaBlockHardLimit.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaBlockHardLimit.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.quotaInodeSoftLimit.description}">
        <label for="{$attributes.quotaInodeSoftLimit.htmlid}">
          {eval var=$attributes.quotaInodeSoftLimit.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaInodeSoftLimit.input}</td>
      <td title="{$attributes.quotaInodeHardLimit.description}">
        <label for="{$attributes.quotaInodeHardLimit.htmlid}">
          {eval var=$attributes.quotaInodeHardLimit.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaInodeHardLimit.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.quota_buttons.description}" colspan="3">
        {eval var=$attributes.quota_buttons.input}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
