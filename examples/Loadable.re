let currentUserIdState =
  Recoil.atom({key: "currentUserId", default: "bloodyowl"});

type user = {
  id: string,
  username: string,
  avatar: string,
};
[@bs.val]
external setTimeout: (unit => unit, int) => unit = "setTimeout";

let getUserMock = (~id) => {
  Promise.exec(resolve => setTimeout(() => {
          resolve({
            id,
            username: "User " ++ id,
            avatar: {j|https://avatars.githubusercontent.com/$id?size=64|j},
          })
        },
        1000));
};

let userState =
  Recoil.asyncSelector({
    key: "currentUser",
    get: ({get}) => {
      let id = get(currentUserIdState);
      getUserMock(~id);
    },
  });

module UserIdPicker = {
  [@react.component]
  let make = () => {
    let currentUserId = Recoil.useRecoilValue(currentUserIdState);

    let (inputValue, setInputValue) = React.useState(() => currentUserId);
    let setCurrentUserIdState = Recoil.useSetRecoilState(currentUserIdState);

    let setUserId = () => {
      setInputValue(_ => "");
      setCurrentUserIdState(_ => inputValue);
    };

    let onChange = event => {
      let value = event->ReactEvent.Form.target##value;
      setInputValue(_ => value);
    };
    <div>
      <p> "Type a GitHub username and press Enter"->React.string </p>
      <input
        type_="text"
        value=inputValue
        onChange
        onKeyDown={event =>
          if (event->ReactEvent.Keyboard.key == "Enter") {
            setUserId();
          }
        }
      />
      <button onClick={_ => setUserId()}> "Set"->React.string </button>
    </div>;
  };
};

module UserCard = {
  [@react.component]
  let make = () => {
    let userLoadable = Recoil.useRecoilValueLoadable(userState);

    React.useEffect1(
      () => {
        /**
         * This shows how to use the toPromise function.
         * It's actually not needed since data will be fetched
         * when component gets mounted.
         */
        Recoil.Loadable.toPromise(userLoadable)
        ->Promise.flatMap(
              value => {
                Promise.resolved(value);
              }
            )
        |> ignore;
        None;
      },
      [|userLoadable|],
    );

    switch (Recoil.Loadable.state(userLoadable)) {
    | loading when loading == Recoil.Loadable.State.loading =>
      "Loading ..."->React.string
    | error when error == Recoil.Loadable.State.hasError =>
      "Error"->React.string
    | _ =>
      let user = Recoil.Loadable.getValue(userLoadable);
      <h1>
        <img src={user.avatar} width="24" height="24" />
        " "->React.string
        <strong> user.username->React.string </strong>
      </h1>;
    };
  };
};

module App = {
  [@react.component]
  let make = () => {
    <> <UserIdPicker /> <UserCard /> </>;
  };
};

ReactDOMRe.renderToElementWithId(
  <Recoil.RecoilRoot> <App /> </Recoil.RecoilRoot>,
  "root",
);
