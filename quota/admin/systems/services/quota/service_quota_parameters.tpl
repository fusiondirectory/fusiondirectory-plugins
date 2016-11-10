<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span><label for="{$attributes.quotaDeviceParameters.htmlid}">{$section}</label></span></legend>
  <div>
  <table>
    <tr>
      <td title="{$attributes.quotaDeviceParameters.description}" colspan="4">
        {eval var=$attributes.quotaDeviceParameters.input}
      </td>
    </tr>
    <tr>
      <td title="{$attributes.quotaDevice.description}">
        <label for="{$attributes.quotaDevice.htmlid}">
          {eval var=$attributes.quotaDevice.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaDevice.input}</td>
      <td title="{$attributes.quotaBlockSize.description}">
        <label for="{$attributes.quotaBlockSize.htmlid}">
          {eval var=$attributes.quotaBlockSize.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaBlockSize.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.quotaDesc.description}">
        <label for="{$attributes.quotaDesc.htmlid}">
          {eval var=$attributes.quotaDesc.label}
        </label>
      </td>
      <td colspan="3">{eval var=$attributes.quotaDesc.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.quotaDeviceParameters_buttons.description}" colspan="4">
        {eval var=$attributes.quotaDeviceParameters_buttons.input}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
