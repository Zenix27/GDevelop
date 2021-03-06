// @flow
import * as React from 'react';
import {
  type EventsFunctionsExtensionWriter,
  type EventsFunctionsExtensionOpener,
} from './Storage';

export type EventsFunctionsExtensionsState = {|
  eventsFunctionsExtensionsError: ?Error,
  loadProjectEventsFunctionsExtensions: (project: ?gdProject) => Promise<void>,
  unloadProjectEventsFunctionsExtensions: (project: gdProject) => void,
  unloadProjectEventsFunctionsExtension: (
    project: gdProject,
    extensionName: string
  ) => void,
  reloadProjectEventsFunctionsExtensions: (
    project: ?gdProject
  ) => Promise<void>,
  getEventsFunctionsExtensionWriter: () => ?EventsFunctionsExtensionWriter,
  getEventsFunctionsExtensionOpener: () => ?EventsFunctionsExtensionOpener,
  ensureLoadFinished: () => Promise<void>,
|};

const defaultState = {
  eventsFunctionsExtensionsError: null,
  loadProjectEventsFunctionsExtensions: () =>
    Promise.reject(new Error('Use a provider')),
  unloadProjectEventsFunctionsExtensions: () => {},
  reloadProjectEventsFunctionsExtensions: () =>
    Promise.reject(new Error('Use a provider')),
  unloadProjectEventsFunctionsExtension: () => {},
  getEventsFunctionsExtensionWriter: () => null,
  getEventsFunctionsExtensionOpener: () => null,
  ensureLoadFinished: () => Promise.reject(new Error('Use a provider')),
};

const EventsFunctionsExtensionsContext = React.createContext<EventsFunctionsExtensionsState>(
  defaultState
);

export default EventsFunctionsExtensionsContext;
