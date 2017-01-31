<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span><label for="{$attributes.fdRenaterPartageServerMailDomain.htmlid}">{$section}</label></span></legend>
  <div>
  <table>
    <tr>
      <td title="{$attributes.fdRenaterPartageServerMailDomain.description}" colspan="4">
        {eval var=$attributes.fdRenaterPartageServerMailDomain.input}
      </td>
    </tr>
    <tr>
      <td title="{$attributes.renaterPartageDomain.description}">
        <label for="{$attributes.renaterPartageDomain.htmlid}">
          {eval var=$attributes.renaterPartageDomain.label}
        </label>
      </td>
      <td>{eval var=$attributes.renaterPartageDomain.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.renaterPartageDomainKey.description}">
        <label for="{$attributes.renaterPartageDomainKey.htmlid}">
          {eval var=$attributes.renaterPartageDomainKey.label}
        </label>
      </td>
      <td>{eval var=$attributes.renaterPartageDomainKey.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.renaterPartageDomainCOS.description}">
        <label for="{$attributes.renaterPartageDomainCOS.htmlid}">
          {eval var=$attributes.renaterPartageDomainCOS.label}
        </label>
      </td>
      <td>{eval var=$attributes.renaterPartageDomainCOS.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.fdRenaterPartageServerMailDomain_buttons.description}" colspan="4">
        {eval var=$attributes.fdRenaterPartageServerMailDomain_buttons.input}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
