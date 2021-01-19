<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span><label for="{$attributes.fdZimbraServerMailDomain.htmlid}">{if $sectionIcon}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}</label></span></legend>
  <div>
  <table>
    <tr>
      <td title="{$attributes.fdZimbraServerMailDomain.description}" colspan="4">
        {eval var=$attributes.fdZimbraServerMailDomain.input}
      </td>
    </tr>
    <tr>
      <td title="{$attributes.zimbraDomain.description}">
        <label for="{$attributes.zimbraDomain.htmlid}">
          {eval var=$attributes.zimbraDomain.label}
        </label>
      </td>
      <td>{eval var=$attributes.zimbraDomain.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.zimbraDomainCOS.description}">
        <label for="{$attributes.zimbraDomainCOS.htmlid}">
          {eval var=$attributes.zimbraDomainCOS.label}
        </label>
      </td>
      <td>{eval var=$attributes.zimbraDomainCOS.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.fdZimbraServerMailDomain_buttons.description}" colspan="4">
        {eval var=$attributes.fdZimbraServerMailDomain_buttons.input}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
