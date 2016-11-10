<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span><label for="{$attributes.goExportEntry.htmlid}">{$section}</label></span></legend>
  <div>
  <table>
    <tr>
      <td title="{$attributes.goExportEntry.description}" colspan="4">
        {eval var=$attributes.goExportEntry.input}
      </td>
    </tr>
    <tr>
      <td title="{$attributes.shareName.description}">
        <label for="{$attributes.shareName.htmlid}">
          {eval var=$attributes.shareName.label}
        </label>
      </td>
      <td>{eval var=$attributes.shareName.input}</td>
      <td title="{$attributes.shareType.description}">
        <label for="{$attributes.shareType.htmlid}">
          {eval var=$attributes.shareType.label}
        </label>
      </td>
      <td>{eval var=$attributes.shareType.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.shareDesc.description}">
        <label for="{$attributes.shareDesc.htmlid}">
          {eval var=$attributes.shareDesc.label}
        </label>
      </td>
      <td>{eval var=$attributes.shareDesc.input}</td>
      <td title="{$attributes.shareCodepage.description}">
        <label for="{$attributes.shareCodepage.htmlid}">
          {eval var=$attributes.shareCodepage.label}
        </label>
      </td>
      <td>{eval var=$attributes.shareCodepage.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.sharePath.description}">
        <label for="{$attributes.sharePath.htmlid}">
          {eval var=$attributes.sharePath.label}
        </label>
      </td>
      <td>{eval var=$attributes.sharePath.input}</td>
      <td title="{$attributes.shareOption.description}">
        <label for="{$attributes.shareOption.htmlid}">
          {eval var=$attributes.shareOption.label}
        </label>
      </td>
      <td>{eval var=$attributes.shareOption.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.goExportEntry_buttons.description}" colspan="4">
        {eval var=$attributes.goExportEntry_buttons.input}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
