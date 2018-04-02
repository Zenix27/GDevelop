// @flow
import React, { Component } from 'react';
import AutoComplete from 'material-ui/AutoComplete';
import Divider from 'material-ui/Divider';
import {
  enumerateLayouts,
  enumerateExternalEvents,
} from '../../../ProjectManager/EnumerateProjectItems';
import { type ParameterFieldProps } from './ParameterFieldProps.flow';

const styles = {
  autoCompleteTextField: {
    minWidth: 300,
  },
};

const fuzzyFilterOrEmpty = (searchText, key) => {
  return !key || AutoComplete.fuzzyFilter(searchText, key);
};

type State = {|
  focused: boolean,
  text: ?string,
|};

export default class ExternalEventsField extends Component<
  ParameterFieldProps,
  State
> {
  state = { focused: false, text: null };

  _description: ?string = undefined;
  _fullList: Array<{ text: string, value: string }> = [];
  _field: ?any;

  constructor(props: ParameterFieldProps) {
    super(props);

    const { parameterMetadata } = this.props;
    this._description = parameterMetadata
      ? parameterMetadata.getDescription()
      : undefined;

    this._loadNamesFrom(props);
  }

  focus() {
    if (this._field) this._field.focus();
  }

  componentWillReceiveProps(newProps: ParameterFieldProps) {
    if (newProps.project !== this.props.project) {
      this._loadNamesFrom(newProps);
    }
  }

  _loadNamesFrom(props: ParameterFieldProps) {
    if (!props.project) {
      return;
    }

    const externalEvents = enumerateExternalEvents(
      props.project
    ).map(externalEvents => ({
      text: externalEvents.getName(),
      value: externalEvents.getName(),
    }));
    const layouts = enumerateLayouts(props.project).map(layout => ({
      text: layout.getName(),
      value: layout.getName(),
    }));
    this._fullList = [
      ...externalEvents,
      { text: '', value: <Divider /> },
      ...layouts,
    ];
  }

  render() {
    return (
      <AutoComplete
        floatingLabelText={this._description}
        fullWidth
        id="external-events-field"
        textFieldStyle={styles.autoCompleteTextField}
        menuProps={{
          maxHeight: 250,
        }}
        searchText={this.state.focused ? this.state.text : this.props.value}
        onFocus={() => {
          this.setState({
            focused: true,
            text: this.props.value,
          });
        }}
        onUpdateInput={value => {
          this.setState({
            focused: true,
            text: value,
          });
        }}
        onBlur={event => {
          this.props.onChange(event.target.value);
          this.setState({
            focused: false,
            text: null,
          });
        }}
        onNewRequest={data => {
          // Note that data may be a string or a {text, value} object.
          if (typeof data === 'string') {
            this.props.onChange(data);
          } else if (typeof data.value === 'string') {
            this.props.onChange(data.value);
          }
        }}
        dataSource={this._fullList}
        filter={fuzzyFilterOrEmpty}
        openOnFocus={!this.props.isInline}
        ref={field => (this._field = field)}
      />
    );
  }
}
